/**
 * Copyright (C) 2010 Johannes Weißl <jargon@molb.org>
 * License: MIT License
 * URL: https://github.com/weisslj/cpp-optparse
 *
 * Modified by AdmiralCurtiss in 2024 to add some features
 * and to make it more C++-like.
 */

#include "OptionParser.h"

#include <algorithm>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "util/number.h"

#if defined(ENABLE_NLS) && ENABLE_NLS
# include <libintl.h>
# define _(s) gettext(s)
#else
# define _(s) ((const char *) (s))
#endif

using namespace std;

namespace optparse {

////////// auxiliary (string) functions { //////////
class str_wrap {
public:
  str_wrap(const string& l, const string& r) : lwrap(l), rwrap(r) {}
  str_wrap(const string& w) : lwrap(w), rwrap(w) {}
  string operator() (const string& s) { return lwrap + s + rwrap; }
  const string lwrap, rwrap;
};
template<typename InputIterator, typename UnaryOperator>
static string str_join_trans(const string& sep, InputIterator begin, InputIterator end, UnaryOperator op) {
  string buf;
  for (InputIterator it = begin; it != end; ++it) {
    if (it != begin)
      buf += sep;
    buf += op(*it);
  }
  return buf;
}
template<class InputIterator>
static string str_join(const string& sep, InputIterator begin, InputIterator end) {
  return str_join_trans(sep, begin, end, str_wrap(""));
}
static string& str_replace(string& s, const string& patt, const string& repl) {
  size_t pos = 0, n = patt.length();
  while (true) {
    pos = s.find(patt, pos);
    if (pos == string::npos)
      break;
    s.replace(pos, n, repl);
    pos += repl.size();
  }
  return s;
}
static string str_replace(const string& s, const string& patt, const string& repl) {
  string tmp = s;
  str_replace(tmp, patt, repl);
  return tmp;
}
static string str_format(const string& str, size_t pre, size_t len, bool running_text = true, bool indent_first = true) {
  string s = str;
  stringstream ss;
  string p;
  len -= 2; // Python seems to not use full length
  if (running_text)
    replace(s.begin(), s.end(), '\n', ' ');
  if (indent_first)
    p = string(pre, ' ');

  size_t pos = 0, linestart = 0;
  size_t line = 0;
  while (true) {
    bool wrap = false;

    size_t new_pos = s.find_first_of(" \n\t", pos);
    if (new_pos == string::npos)
      break;
    if (s[new_pos] == '\n') {
      pos = new_pos + 1;
      wrap = true;
    }
    if (line == 1)
      p = string(pre, ' ');
    if (wrap || new_pos + pre > linestart + len) {
      ss << p << s.substr(linestart, pos - linestart - 1) << endl;
      linestart = pos;
      line++;
    }
    pos = new_pos + 1;
  }
  ss << p << s.substr(linestart) << endl;
  return ss.str();
}
static unsigned int cols() {
  unsigned int n = 80;
#ifndef _WIN32
  const char *s = getenv("COLUMNS");
  if (s)
    istringstream(s) >> n;
#endif
  return n;
}
static string basename(const string& s) {
  string b = s;
  size_t i = b.find_last_not_of("/\\");
  if (i == string::npos) {
    if (b[0] == '/' || b[0] == '\\')
      b.erase(1);
    return b;
  }
  b.erase(i+1, b.length()-i-1);
  i = b.find_last_of("/\\");
  if (i != string::npos)
    b.erase(0, i+1);
  return b;
}
static std::string to_string(const Value::VariantT& value) {
  if (std::holds_alternative<bool>(value)) {
    return std::get<bool>(value) ? "true" : "false";
  }
  if (std::holds_alternative<int64_t>(value)) {
    return std::format("{}", std::get<int64_t>(value));
  }
  if (std::holds_alternative<double>(value)) {
    return std::format("{}", std::get<double>(value));
  }
  if (std::holds_alternative<std::complex<double>>(value)) {
    auto& c = std::get<std::complex<double>>(value);
    return std::format("{}{:+}j", c.real(), c.imag());
  }
  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
  }
  if (std::holds_alternative<std::vector<int64_t>>(value)) {
    auto& vec = std::get<std::vector<int64_t>>(value);
    std::string s;
    s.push_back('[');
    if (!vec.empty()) {
      std::format_to(std::back_inserter(s), "{}", vec[0]);
      for (size_t i = 1; i < vec.size(); ++i) {
        s.push_back(',');
        s.push_back(' ');
        std::format_to(std::back_inserter(s), "{}", vec[i]);
      }
    }
    s.push_back(']');
    return s;
  }
  if (std::holds_alternative<std::vector<double>>(value)) {
    auto& vec = std::get<std::vector<double>>(value);
    std::string s;
    s.push_back('[');
    if (!vec.empty()) {
      std::format_to(std::back_inserter(s), "{}", vec[0]);
      for (size_t i = 1; i < vec.size(); ++i) {
        s.push_back(',');
        s.push_back(' ');
        std::format_to(std::back_inserter(s), "{}", vec[i]);
      }
    }
    s.push_back(']');
    return s;
  }
  if (std::holds_alternative<std::vector<std::complex<double>>>(value)) {
    auto& vec = std::get<std::vector<std::complex<double>>>(value);
    std::string s;
    s.push_back('[');
    if (!vec.empty()) {
      std::format_to(std::back_inserter(s), "{}{:+}j", vec[0].real(), vec[0].imag());
      for (size_t i = 1; i < vec.size(); ++i) {
        s.push_back(',');
        s.push_back(' ');
        std::format_to(std::back_inserter(s), "{}{:+}j", vec[i].real(), vec[i].imag());
      }
    }
    s.push_back(']');
    return s;
  }
  if (std::holds_alternative<std::vector<std::string>>(value)) {
    auto& vec = std::get<std::vector<std::string>>(value);
    std::string s;
    s.push_back('[');
    if (!vec.empty()) {
      s.append(vec[0]);
      for (size_t i = 1; i < vec.size(); ++i) {
        s.push_back(',');
        s.push_back(' ');
        s.append(vec[i]);
      }
    }
    s.push_back(']');
    return s;
  }
  return "";
}
////////// } auxiliary (string) functions //////////


////////// class OptionContainer { //////////
Option& OptionContainer::add_option(const string& opt) {
  const string tmp[1] = { opt };
  return add_option(vector<string>(&tmp[0], &tmp[1]));
}
Option& OptionContainer::add_option(const string& opt1, const string& opt2) {
  const string tmp[2] = { opt1, opt2 };
  return add_option(vector<string>(&tmp[0], &tmp[2]));
}
Option& OptionContainer::add_option(const string& opt1, const string& opt2, const string& opt3) {
  const string tmp[3] = { opt1, opt2, opt3 };
  return add_option(vector<string>(&tmp[0], &tmp[3]));
}
Option& OptionContainer::add_option(const vector<string>& v) {
  _opts.resize(_opts.size()+1, Option(get_parser()));
  Option& option = _opts.back();
  string dest_fallback;
  for (vector<string>::const_iterator it = v.begin(); it != v.end(); ++it) {
    if (it->substr(0,2) == "--") {
      const string s = it->substr(2);
      if (option.dest() == "")
        option.dest(str_replace(s, "-", "_"));
      option._long_opts.insert(s);
      _optmap_l[s] = &option;
    } else {
      const string s = it->substr(1,1);
      if (dest_fallback == "")
        dest_fallback = s;
      option._short_opts.insert(s);
      _optmap_s[s] = &option;
    }
  }
  if (option.dest() == "")
    option.dest(dest_fallback);
  return option;
}
string OptionContainer::format_option_help(unsigned int indent /* = 2 */) const {
  stringstream ss;

  if (_opts.empty())
    return ss.str();

  for (list<Option>::const_iterator it = _opts.begin(); it != _opts.end(); ++it) {
    if (it->help() != SUPPRESS_HELP)
      ss << it->format_help(indent);
  }

  return ss.str();
}
////////// } class OptionContainer //////////

////////// class OptionParser { //////////
OptionParser::OptionParser()
  : OptionContainer()
  , _usage(_("%prog [options]"))
  , _add_help_option(true)
  , _add_version_option(true)
  , _interspersed_args(true)
  , _values(this) {}

OptionParser& OptionParser::set_defaults(const std::string& dest, Value::VariantT val) {
  _defaults.insert_or_assign(dest, std::move(val));
  return *this;
}

OptionParser& OptionParser::add_option_group(const OptionGroup& group) {
  for (list<Option>::const_iterator oit = group._opts.begin(); oit != group._opts.end(); ++oit) {
    const Option& option = *oit;
    for (set<string>::const_iterator it = option._short_opts.begin(); it != option._short_opts.end(); ++it)
      _optmap_s[*it] = &option;
    for (set<string>::const_iterator it = option._long_opts.begin(); it != option._long_opts.end(); ++it)
      _optmap_l[*it] = &option;
  }
  _groups.push_back(&group);
  return *this;
}

const Option& OptionParser::lookup_short_opt(const string& opt) const {
  auto it = _optmap_s.find(opt);
  if (it == _optmap_s.end())
    error(_("no such option") + string(": -") + opt);
  return *it->second;
}

void OptionParser::handle_short_opt(const string& opt, const string& arg) {

  _remaining.pop_front();
  string value;

  const Option& option = lookup_short_opt(opt);
  if (option._nargs == 1) {
    value = arg.substr(2);
    if (value == "") {
      if (_remaining.empty())
        error("-" + opt + " " + _("option requires 1 argument"));
      value = _remaining.front();
      _remaining.pop_front();
    }
  } else {
    if (arg.length() > 2)
      _remaining.push_front(string("-") + arg.substr(2));
  }

  process_opt(option, string("-") + opt, value);
}

const Option& OptionParser::lookup_long_opt(const string& opt) const {

  list<string> matching;
  for (auto it = _optmap_l.begin(); it != _optmap_l.end(); ++it) {
    if (it->first.compare(0, opt.length(), opt) == 0) {
      matching.push_back(it->first);
      if (it->first.length() == opt.length())
        break;
    }
  }
  if (matching.size() > 1) {
    string x = str_join_trans(", ", matching.begin(), matching.end(), str_wrap("--", ""));
    error(_("ambiguous option") + string(": --") + opt + " (" + x + "?)");
  }
  if (matching.size() == 0)
    error(_("no such option") + string(": --") + opt);

  return *_optmap_l.find(matching.front())->second;
}

void OptionParser::handle_long_opt(const string& optstr) {

  _remaining.pop_front();
  string opt, value;

  size_t delim = optstr.find("=");
  if (delim != string::npos) {
    opt = optstr.substr(0, delim);
    value = optstr.substr(delim+1);
  } else
    opt = optstr;

  const Option& option = lookup_long_opt(opt);
  if (option._nargs == 1 && delim == string::npos) {
    if (!_remaining.empty()) {
      value = _remaining.front();
      _remaining.pop_front();
    }
  }

  if (option._nargs == 1 && value == "")
    error("--" + opt + " " + _("option requires 1 argument"));

  process_opt(option, string("--") + opt, value);
}

Values& OptionParser::parse_args(const int argc, char const* const* const argv) {
  if (prog() == "")
    prog(basename(argv[0]));
  return parse_args(&argv[1], &argv[argc]);
}
Values& OptionParser::parse_args(const vector<string>& v) {

  _remaining.assign(v.begin(), v.end());

  if (add_help_option()) {
    add_option("-h", "--help") .action(ActionType::Help) .help(_("Show this help message and exit."));
    _opts.splice(_opts.begin(), _opts, --(_opts.end()));
  }
  if (add_version_option() && version() != "") {
    add_option("--version") .action(ActionType::Version) .help(_("Show the program's version number and exit."));
    _opts.splice(_opts.begin(), _opts, --(_opts.end()));
  }

  while (!_remaining.empty()) {
    const string arg = _remaining.front();

    if (arg == "--") {
      _remaining.pop_front();
      break;
    }

    if (arg.substr(0,2) == "--") {
      handle_long_opt(arg.substr(2));
    } else if (arg.substr(0,1) == "-" && arg.length() > 1) {
      handle_short_opt(arg.substr(1,1), arg);
    } else {
      _remaining.pop_front();
      _leftover.push_back(arg);
      if (!interspersed_args())
        break;
    }
  }
  while (!_remaining.empty()) {
    const string arg = _remaining.front();
    _remaining.pop_front();
    _leftover.push_back(arg);
  }

  const auto handle_default_val = [&](const Option& o) -> void {
    if (_values.get(o.dest()) == nullptr) {
      const auto& def = o.get_default();
      if (!std::holds_alternative<std::monostate>(def))
        _values.set(o.dest(), Value(this, def, false));
      else if (o.action() == ActionType::StoreTrue)
        _values.set(o.dest(), Value(this, false, false));
      else if (o.action() == ActionType::StoreFalse)
        _values.set(o.dest(), Value(this, true, false));
      else if (o.action() == ActionType::Count)
        _values.set(o.dest(), Value(this, int64_t(0), false));
    }
  };

  for (list<Option>::const_iterator it = _opts.begin(); it != _opts.end(); ++it) {
    handle_default_val(*it);
  }

  for (list<OptionGroup const*>::iterator group_it = _groups.begin(); group_it != _groups.end(); ++group_it) {
    for (list<Option>::const_iterator it = (*group_it)->_opts.begin(); it != (*group_it)->_opts.end(); ++it) {
      handle_default_val(*it);
    }
  }

  return _values;
}

void OptionParser::process_opt(const Option& o, const string& opt, const string& value) {
  if (o.action() == ActionType::Store) {
    _values.set(o.dest(), o.make_value_from_string(opt, this, value, true));
  }
  else if (o.action() == ActionType::StoreConst) {
    _values.set(o.dest(), o.make_value_from_string(opt, this, o.get_const(), true));
  }
  else if (o.action() == ActionType::StoreTrue) {
    _values.set(o.dest(), Value(this, true, true));
  }
  else if (o.action() == ActionType::StoreFalse) {
    _values.set(o.dest(), Value(this, false, true));
  }
  else if (o.action() == ActionType::Append) {
    _values.append(o.dest(), o.make_value_from_string(opt, this, value, true));
  }
  else if (o.action() == ActionType::AppendConst) {
    _values.append(o.dest(), o.make_value_from_string(opt, this, o.get_const(), true));
  }
  else if (o.action() == ActionType::Count) {
    int64_t count = 0;
    const Value* v = _values.get(o.dest());
    if (v) {
      auto ints = v->integers();
      if (!ints.empty()) {
        count = ints.front();
      }
    }
    _values.set(o.dest(), Value(this, int64_t(count + 1), true));
  }
  else if (o.action() == ActionType::Help) {
    print_help();
    std::exit(0);
  }
  else if (o.action() == ActionType::Version) {
    print_version();
    std::exit(0);
  }
  else if (o.action() == ActionType::Callback && o.callback()) {
    (*o.callback())(o, opt, o.make_value_from_string(opt, this, value, true), *this);
  }
}

string OptionParser::format_help() const {
  stringstream ss;

  if (usage() != SUPPRESS_USAGE)
    ss << get_usage() << endl;

  if (description() != "")
    ss << str_format(description(), 0, cols(), false) << endl;

  ss << _("Options") << ":" << endl;
  ss << format_option_help();

  for (list<OptionGroup const*>::const_iterator it = _groups.begin(); it != _groups.end(); ++it) {
    const OptionGroup& group = **it;
    ss << endl << "  " << group.title() << ":" << endl;
    if (group.description() != "") {
      unsigned int malus = 4; // Python seems to not use full length
      ss << str_format(group.description(), 4, cols() - malus) << endl;
    }
    ss << group.format_option_help(4);
  }

  if (epilog() != "")
    ss << endl << str_format(epilog(), 0, cols(), false);

  return ss.str();
}
void OptionParser::print_help() const {
  cout << format_help();
}

void OptionParser::set_usage(const string& u) {
  string lower = u;
  transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  if (lower.compare(0, 7, "usage: ") == 0)
    _usage = u.substr(7);
  else
    _usage = u;
}
string OptionParser::format_usage(const string& u) const {
  stringstream ss;
  ss << _("Usage") << ": " << u << endl;
  return ss.str();
}
string OptionParser::get_usage() const {
  if (usage() == SUPPRESS_USAGE)
    return string("");
  return format_usage(str_replace(usage(), "%prog", prog()));
}
void OptionParser::print_usage(ostream& out) const {
  string u = get_usage();
  if (u != "")
    out << u << endl;
}
void OptionParser::print_usage() const {
  print_usage(cout);
}

string OptionParser::get_version() const {
  return str_replace(_version, "%prog", prog());
}
void OptionParser::print_version(ostream& out) const {
  out << get_version() << endl;
}
void OptionParser::print_version() const {
  print_version(cout);
}

void OptionParser::exit() const {
  std::exit(-2);
}
void OptionParser::error(const string& msg) const {
  cerr << format_help() << endl;
  cerr << _("error") << ": " << msg << endl;
  exit();
}
////////// } class OptionParser //////////

////////// class Value { //////////
Value::Value(const OptionParser* parser, VariantT v, bool set_by_user)
  : m_parser_ref(parser)
  , m_value(std::move(v))
  , m_set_by_user(set_by_user) {}

Value::Value(const Value& other) = default;
Value::Value(Value&& other) = default;
Value& Value::operator=(const Value& other) = default;
Value& Value::operator=(Value&& other) = default;
Value::~Value() = default;

bool Value::flag() const {
  if (std::holds_alternative<bool>(m_value))
    return std::get<bool>(m_value);
  m_parser_ref->error("internal error");
}

std::span<const int64_t> Value::integers() const {
  if (std::holds_alternative<int64_t>(m_value))
    return std::span<const int64_t>(&std::get<int64_t>(m_value), size_t(1));
  if (std::holds_alternative<std::vector<int64_t>>(m_value))
    return std::get<std::vector<int64_t>>(m_value);
  m_parser_ref->error("internal error");
}

std::span<const double> Value::floats() const {
  if (std::holds_alternative<double>(m_value))
    return std::span<const double>(&std::get<double>(m_value), size_t(1));
  if (std::holds_alternative<std::vector<double>>(m_value))
    return std::get<std::vector<double>>(m_value);
  m_parser_ref->error("internal error");
}

std::span<const std::complex<double>> Value::complexes() const {
  if (std::holds_alternative<std::complex<double>>(m_value))
    return std::span<const std::complex<double>>(&std::get<std::complex<double>>(m_value), size_t(1));
  if (std::holds_alternative<std::vector<std::complex<double>>>(m_value))
    return std::get<std::vector<std::complex<double>>>(m_value);
  m_parser_ref->error("internal error");
}

std::span<const std::string> Value::strings() const {
  if (std::holds_alternative<std::string>(m_value))
    return std::span<const std::string>(&std::get<std::string>(m_value), size_t(1));
  if (std::holds_alternative<std::vector<std::string>>(m_value))
    return std::get<std::vector<std::string>>(m_value);
  m_parser_ref->error("internal error");
}

int64_t Value::first_integer() const {
  auto v = integers();
  if (v.empty()) {
    m_parser_ref->error("internal error");
  }
  return v.front();
}

double Value::first_float() const {
  auto v = floats();
  if (v.empty()) {
    m_parser_ref->error("internal error");
  }
  return v.front();
}

std::complex<double> Value::first_complex() const {
  auto v = complexes();
  if (v.empty()) {
    m_parser_ref->error("internal error");
  }
  return v.front();
}

const std::string& Value::first_string() const {
  auto v = strings();
  if (v.empty()) {
    m_parser_ref->error("internal error");
  }
  return v.front();
}

bool Value::is_set_by_user() const {
  return m_set_by_user;
}
////////// } class Value //////////

////////// class Values { //////////
Values::Values(const OptionParser* parser)
  : m_parser_ref(parser)
  , m_values_per_option() {}

Values::Values(const Values& other) = default;
Values::Values(Values&& other) = default;
Values& Values::operator=(const Values& other) = default;
Values& Values::operator=(Values&& other) = default;
Values::~Values() = default;

const Value& Values::operator[](std::string_view key) const {
  const Value* value = get(key);
  if (!value) {
    m_parser_ref->error("internal error");
  }
  return *value;
}

const Value* Values::get(std::string_view key) const {
  auto it = m_values_per_option.find(key);
  if (it == m_values_per_option.end()) {
    return nullptr;
  }
  return &it->second;
}

void Values::set(const std::string& key, Value value) {
  m_values_per_option.insert_or_assign(key, std::move(value));
}

void Values::append(const std::string& key, Value value) {
  auto it = m_values_per_option.find(key);
  if (it == m_values_per_option.end()) {
    set(key, std::move(value));
    return;
  }

  auto& merged_value = it->second.m_value;

  // expand to vector if needed
  if (std::holds_alternative<int64_t>(merged_value)) {
    int64_t oldval = std::get<int64_t>(merged_value);
    std::vector<int64_t> newval;
    newval.push_back(oldval);
    merged_value = std::move(newval);
  }
  else if (std::holds_alternative<double>(merged_value)) {
    double oldval = std::get<double>(merged_value);
    std::vector<double> newval;
    newval.push_back(oldval);
    merged_value = std::move(newval);
  }
  else if (std::holds_alternative<std::complex<double>>(merged_value)) {
    std::complex<double> oldval = std::get<std::complex<double>>(merged_value);
    std::vector<std::complex<double>> newval;
    newval.push_back(oldval);
    merged_value = std::move(newval);
  }
  else if (std::holds_alternative<std::string>(merged_value)) {
    std::string oldval = std::move(std::get<std::string>(merged_value));
    std::vector<std::string> newval;
    newval.push_back(std::move(oldval));
    merged_value = std::move(newval);
  }

  // merge
  if (std::holds_alternative<std::monostate>(merged_value)) {
    merged_value = std::move(value.m_value);
  }
  else if (std::holds_alternative<std::vector<int64_t>>(merged_value)) {
    auto& lhs = std::get<std::vector<int64_t>>(merged_value);
    if (std::holds_alternative<int64_t>(value.m_value)) {
      lhs.push_back(std::get<int64_t>(value.m_value));
    }
    else if (std::holds_alternative<std::vector<int64_t>>(value.m_value)) {
      auto& rhs = std::get<std::vector<int64_t>>(value.m_value);
      lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    }
    else {
      m_parser_ref->error("incompatible types");
    }
  }
  else if (std::holds_alternative<std::vector<double>>(merged_value)) {
    auto& lhs = std::get<std::vector<double>>(merged_value);
    if (std::holds_alternative<double>(value.m_value)) {
      lhs.push_back(std::get<double>(value.m_value));
    }
    else if (std::holds_alternative<std::vector<double>>(value.m_value)) {
      auto& rhs = std::get<std::vector<double>>(value.m_value);
      lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    }
    else {
      m_parser_ref->error("incompatible types");
    }
  }  
  else if (std::holds_alternative<std::vector<std::complex<double>>>(merged_value)) {
    auto& lhs = std::get<std::vector<std::complex<double>>>(merged_value);
    if (std::holds_alternative<std::complex<double>>(value.m_value)) {
      lhs.push_back(std::get<std::complex<double>>(value.m_value));
    }
    else if (std::holds_alternative<std::vector<std::complex<double>>>(value.m_value)) {
      auto& rhs = std::get<std::vector<std::complex<double>>>(value.m_value);
      lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    }
    else {
      m_parser_ref->error("incompatible types");
    }
  }  
  else if (std::holds_alternative<std::vector<std::string>>(merged_value)) {
    auto& lhs = std::get<std::vector<std::string>>(merged_value);
    if (std::holds_alternative<std::string>(value.m_value)) {
      lhs.push_back(std::move(std::get<std::string>(value.m_value)));
    }
    else if (std::holds_alternative<std::vector<std::string>>(value.m_value)) {
      auto& rhs = std::get<std::vector<std::string>>(value.m_value);
      lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    }
    else {
      m_parser_ref->error("incompatible types");
    }
  }
  else {
    m_parser_ref->error("incompatible types");
  }

  it->second.m_set_by_user |= value.m_set_by_user;
}
////////// } class Values //////////

////////// class Option { //////////
Value Option::make_value_from_string(const std::string& opt, const OptionParser* parser, std::string_view val, bool set_by_user) const {
  if (type() == DataType::Int) {
    auto t = HyoutaUtils::NumberUtils::ParseInt64(val);
    if (!t) {
      stringstream err;
      err << _("option") << " " << opt << ": " << _("invalid integer value") << ": '" << val << "'";
      parser->error(err.str());
    }
    return Value(parser, *t, set_by_user);
  }
  else if (type() == DataType::Float) {
    double t;
    istringstream ss{std::string(val)};
    if (!(ss >> t)) {
      stringstream err;
      err << _("option") << " " << opt << ": " << _("invalid floating-point value") << ": '" << val << "'";
      parser->error(err.str());
    }
    return Value(parser, t, set_by_user);
  }
  else if (type() == DataType::Choice) {
    if (find(choices().begin(), choices().end(), val) == choices().end()) {
      list<string> tmp = choices();
      transform(tmp.begin(), tmp.end(), tmp.begin(), str_wrap("'"));
      stringstream err;
      err << _("option") << " " << opt << ": " << _("invalid choice") << ": '" << val << "'"
          << " (" << _("choose from") << " " << str_join(", ", tmp.begin(), tmp.end()) << ")";
      parser->error(err.str());
    }
  }
  else if (type() == DataType::Complex) {
    complex<double> t;
    istringstream ss{std::string(val)};
    if (!(ss >> t)) {
      stringstream err;
      err << _("option") << " " << opt << ": " << _("invalid complex value") << ": '" << val << "'";
      parser->error(err.str());
    }
    return Value(parser, t, set_by_user);
  }

  return Value(parser, std::string(val), set_by_user);
}

string Option::format_option_help(unsigned int indent /* = 2 */) const {

  string mvar_short, mvar_long;
  if (nargs() == 1) {
    string mvar = metavar();
    if (mvar == "") {
      mvar = dest();
      transform(mvar.begin(), mvar.end(), mvar.begin(), ::toupper);
     }
    mvar_short = " " + mvar;
    mvar_long = "=" + mvar;
  }

  stringstream ss;
  ss << string(indent, ' ');

  if (!_short_opts.empty()) {
    ss << str_join_trans(", ", _short_opts.begin(), _short_opts.end(), str_wrap("-", mvar_short));
    if (!_long_opts.empty())
      ss << ", ";
  }
  if (!_long_opts.empty())
    ss << str_join_trans(", ", _long_opts.begin(), _long_opts.end(), str_wrap("--", mvar_long));

  return ss.str();
}

string Option::format_help(unsigned int indent /* = 2 */) const {
  stringstream ss;
  string h = format_option_help(indent);
  unsigned int width = cols();
  unsigned int opt_width = min(width*3/10, 36u);
  bool indent_first = false;
  ss << h;
  // if the option list is too long, start a new paragraph
  if (h.length() >= (opt_width-1)) {
    ss << endl;
    indent_first = true;
  } else {
    ss << string(opt_width - h.length(), ' ');
    if (help() == "")
      ss << endl;
  }
  if (help() != "") {
    const auto& def = get_default();
    string help_str = (!std::holds_alternative<std::monostate>(def)) ? str_replace(help(), "%default", to_string(def)) : help();
    ss << str_format(help_str, opt_width, width, false, indent_first);
  }
  return ss.str();
}

Option& Option::action(ActionType a) {
  _action = a;
  if (a == ActionType::StoreConst || a == ActionType::StoreTrue || a == ActionType::StoreFalse ||
      a == ActionType::AppendConst || a == ActionType::Count || a == ActionType::Help ||
      a == ActionType::Version) {
    nargs(0);
  } else if (a == ActionType::Callback) {
    nargs(0);
    type(DataType::None);
  }
  return *this;
}


Option& Option::type(DataType t) {
  _type = t;
  nargs((t == DataType::None) ? 0 : 1);
  return *this;
}

Option& Option::set_default(Value::VariantT d) {
  _default = std::move(d);
  return *this;
}

const Value::VariantT& Option::get_default() const {
  auto it = _parser._defaults.find(dest());
  if (it != _parser._defaults.end())
    return it->second;
  else
    return _default;
}
////////// } class Option //////////

}

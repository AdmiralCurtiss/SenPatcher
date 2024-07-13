/**
 * Copyright (C) 2010 Johannes Weißl <jargon@molb.org>
 * License: MIT License
 * URL: https://github.com/weisslj/cpp-optparse
 *
 * Modified by AdmiralCurtiss in 2024 to add some features
 * and to make it more C++-like.
 */

#ifndef OPTIONPARSER_H_
#define OPTIONPARSER_H_

#include <complex>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace optparse {

class OptionParser;
class OptionGroup;
class Option;
class Values;
class Value;
class Callback;

enum class ActionType : uint8_t {
    Store,
    StoreConst,
    StoreTrue,
    StoreFalse,
    Append,
    AppendConst,
    Count,
    Callback,
    Help,
    Version,
};

enum class DataType : uint8_t {
    None,
    String,
    Int,
    Choice,
    Float,
    Complex,
};

const char* const SUPPRESS_HELP = "SUPPRESS" "HELP";
const char* const SUPPRESS_USAGE = "SUPPRESS" "USAGE";

class Value {
public:
  // indicates a command line flag without a value, usually given via StoreTrue or StoreFalse
  bool flag() const;

  std::span<const int64_t> integers() const;
  std::span<const double> floats() const;
  std::span<const std::complex<double>> complexes() const;
  std::span<const std::string> strings() const;

  int64_t first_integer() const;
  double first_float() const;
  std::complex<double> first_complex() const;
  const std::string& first_string() const;

  bool is_set_by_user() const;

  using VariantT = std::variant<std::monostate,
                                bool,
                                int64_t,
                                double,
                                std::complex<double>,
                                std::string,
                                std::vector<int64_t>,
                                std::vector<double>,
                                std::vector<std::complex<double>>,
                                std::vector<std::string>>;
  Value(const OptionParser* parser, VariantT v, bool set_by_user);
  Value(const Value& other);
  Value(Value&& other);
  Value& operator=(const Value& other);
  Value& operator=(Value&& other);
  ~Value();

private:
  const OptionParser* m_parser_ref;
  VariantT m_value;
  bool m_set_by_user;

  friend class Values;
};

class Values {
public:
  Values(const OptionParser* parser);
  Values(const Values& other);
  Values(Values&& other);
  Values& operator=(const Values& other);
  Values& operator=(Values&& other);
  ~Values();

  const Value& operator[](std::string_view key) const;
  const Value* get(std::string_view key) const;

  void set(const std::string& key, Value value);
  void append(const std::string& key, Value value);

private:
  struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
  };

  const OptionParser* m_parser_ref;
  std::unordered_map<std::string, Value, string_hash, std::equal_to<>> m_values_per_option;
};

class Option {
  public:
    Option(const OptionParser& p) :
      _parser(p), _action(ActionType::Store), _type(DataType::String), _nargs(1), _callback(0) {}
    virtual ~Option() {}

    Option& action(ActionType a);
    Option& type(DataType t);
    Option& dest(const std::string& d) { _dest = d; return *this; }
    Option& set_default(Value::VariantT d);
    Option& nargs(size_t n) { _nargs = n; return *this; }
    Option& set_const(const std::string& c) { _const = c; return *this; }
    template<typename InputIterator>
    Option& choices(InputIterator begin, InputIterator end) {
      _choices.assign(begin, end); type(DataType::Choice); return *this;
    }
    Option& choices(std::initializer_list<std::string> ilist) {
      _choices.assign(ilist); type(DataType::Choice); return *this;
    }
    Option& help(const std::string& h) { _help = h; return *this; }
    Option& metavar(const std::string& m) { _metavar = m; return *this; }
    Option& callback(Callback& c) { _callback = &c; return *this; }

    ActionType action() const { return _action; }
    DataType type() const { return _type; }
    const std::string& dest() const { return _dest; }
    const Value::VariantT& get_default() const;
    size_t nargs() const { return _nargs; }
    const std::string& get_const() const { return _const; }
    const std::list<std::string>& choices() const { return _choices; }
    const std::string& help() const { return _help; }
    const std::string& metavar() const { return _metavar; }
    Callback* callback() const { return _callback; }

  private:
    Value make_value_from_string(const std::string& opt, const OptionParser* parser, std::string_view val, bool set_by_user) const;
    std::string format_option_help(unsigned int indent = 2) const;
    std::string format_help(unsigned int indent = 2) const;

    const OptionParser& _parser;

    std::set<std::string> _short_opts;
    std::set<std::string> _long_opts;

    ActionType _action;
    DataType _type;
    std::string _dest;
    Value::VariantT _default = std::monostate();
    size_t _nargs;
    std::string _const;
    std::list<std::string> _choices;
    std::string _help;
    std::string _metavar;
    Callback* _callback;

    friend class OptionContainer;
    friend class OptionParser;
};

class OptionContainer {
  public:
    OptionContainer(const std::string& d = "") : _description(d) {}
    virtual ~OptionContainer() {}

    virtual OptionContainer& description(const std::string& d) { _description = d; return *this; }
    virtual const std::string& description() const { return _description; }

    Option& add_option(const std::string& opt);
    Option& add_option(const std::string& opt1, const std::string& opt2);
    Option& add_option(const std::string& opt1, const std::string& opt2, const std::string& opt3);
    Option& add_option(const std::vector<std::string>& opt);

    std::string format_option_help(unsigned int indent = 2) const;

  protected:
    std::string _description;

    std::list<Option> _opts;
    std::map<std::string, Option const*> _optmap_s;
    std::map<std::string, Option const*> _optmap_l;

  private:
    virtual const OptionParser& get_parser() = 0;
};

class OptionParser : public OptionContainer {
  public:
    OptionParser();
    virtual ~OptionParser() {}

    OptionParser& usage(const std::string& u) { set_usage(u); return *this; }
    OptionParser& version(const std::string& v) { _version = v; return *this; }
    OptionParser& description(const std::string& d) { _description = d; return *this; }
    OptionParser& add_help_option(bool h) { _add_help_option = h; return *this; }
    OptionParser& add_version_option(bool v) { _add_version_option = v; return *this; }
    OptionParser& prog(const std::string& p) { _prog = p; return *this; }
    OptionParser& epilog(const std::string& e) { _epilog = e; return *this; }
    OptionParser& set_defaults(const std::string& dest, Value::VariantT val);
    OptionParser& enable_interspersed_args() { _interspersed_args = true; return *this; }
    OptionParser& disable_interspersed_args() { _interspersed_args = false; return *this; }
    OptionParser& add_option_group(const OptionGroup& group);

    const std::string& usage() const { return _usage; }
    const std::string& version() const { return _version; }
    const std::string& description() const { return _description; }
    bool add_help_option() const { return _add_help_option; }
    bool add_version_option() const { return _add_version_option; }
    const std::string& prog() const { return _prog; }
    const std::string& epilog() const { return _epilog; }
    bool interspersed_args() const { return _interspersed_args; }

    Values& parse_args(int argc, char const* const* argv);
    Values& parse_args(const std::vector<std::string>& args);
    template<typename InputIterator>
    Values& parse_args(InputIterator begin, InputIterator end) {
      return parse_args(std::vector<std::string>(begin, end));
    }

    const std::list<std::string>& args() const { return _leftover; }
    std::vector<std::string> args() {
      return std::vector<std::string>(_leftover.begin(), _leftover.end());
    }

    std::string format_help() const;
    void print_help() const;

    void set_usage(const std::string& u);
    std::string get_usage() const;
    void print_usage(std::ostream& out) const;
    void print_usage() const;

    std::string get_version() const;
    void print_version(std::ostream& out) const;
    void print_version() const;

    void error(const std::string& msg) const;
    void exit() const;

  private:
    const OptionParser& get_parser() { return *this; }
    const Option& lookup_short_opt(const std::string& opt) const;
    const Option& lookup_long_opt(const std::string& opt) const;

    void handle_short_opt(const std::string& opt, const std::string& arg);
    void handle_long_opt(const std::string& optstr);

    void process_opt(const Option& option, const std::string& opt, const std::string& value);

    std::string format_usage(const std::string& u) const;

    std::string _usage;
    std::string _version;
    bool _add_help_option;
    bool _add_version_option;
    std::string _prog;
    std::string _epilog;
    bool _interspersed_args;

    Values _values;

    std::unordered_map<std::string, Value::VariantT> _defaults;
    std::list<OptionGroup const*> _groups;

    std::list<std::string> _remaining;
    std::list<std::string> _leftover;

    friend class Option;
};

class OptionGroup : public OptionContainer {
  public:
    OptionGroup(const OptionParser& p, const std::string& t, const std::string& d = "") :
      OptionContainer(d), _parser(p), _title(t) {}
    virtual ~OptionGroup() {}

    OptionGroup& title(const std::string& t) { _title = t; return *this; }
    const std::string& title() const { return _title; }

  private:
    const OptionParser& get_parser() { return _parser; }

    const OptionParser& _parser;
    std::string _title;

  friend class OptionParser;
};

class Callback {
public:
  virtual void operator() (const Option& option, const std::string& opt, const Value& val, const OptionParser& parser) = 0;
  virtual ~Callback() {}
};

}

#endif

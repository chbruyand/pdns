/**
 * This file is part of PowerDNS or dnsdist.
 * Copyright -- PowerDNS.COM B.V. and its contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * In addition, for the avoidance of any doubt, permission is granted to
 * link this program with OpenSSL and to (re)distribute the binaries
 * produced as the result of such linking.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "logging.hh"

namespace Logging
{

  std::shared_ptr<Logger> Logger::getptr()
  {
    return shared_from_this();
  }

  bool Logger::enabled()
  {
    return true;
  }

  void Logger::info(const std::string& msg)
  {
    logMessage(msg, boost::none);
  }

  void Logger::logMessage(const std::string& msg, boost::optional<const std::string> err)
  {
    if (_level > _verbosity) {
      return ;
    }
    auto entry = std::unique_ptr<Entry>(new Entry());
    entry->level = _level;
    entry->name = _name;
    entry->message = msg;
    entry->error = err;
    auto parent = _parent;
    entry->values.insert(_values.begin(), _values.end());
    while (parent) {
      entry->values.insert(parent->_values.begin(), parent->_values.end());
      parent = parent->_parent;
    }
    _callback(std::move(entry));
  }

  void Logger::error(const std::string& err, const std::string& msg)
  {
    logMessage(msg, err);
  }

  std::shared_ptr<Logr::Logger> Logger::v(size_t level)
  {
    auto res = std::make_shared<Logger>(getptr(), boost::none, level + _level, _callback);
    res->setVerbosity(getVerbosity());
    return res;
  }

  std::shared_ptr<Logr::Logger> Logger::withValues(const std::string& key, const Logr::Loggable& value)
  {
    auto res = std::make_shared<Logger>(getptr(), _name.get(), _level, _callback);
    res->_values.insert({key, value.to_string()});
    res->setVerbosity(getVerbosity());
    return res;
  }

  template struct Loggable<DNSName>;
  template struct Loggable<ComboAddress>;
  template struct Loggable<std::string>;

  template <>
  std::string Loggable<DNSName>::to_string() const {
    return _t.toLogString();
  }
  template <>
  std::string Loggable<ComboAddress>::to_string() const {
    return _t.toLogString();
  }
  template <>
  std::string Loggable<std::string>::to_string() const {
    return _t;
  }

  std::shared_ptr<Logr::Logger> Logger::withName(const std::string& name)
  {
    std::shared_ptr<Logger> res;
    if (_name) {
      res = std::make_shared<Logger>(getptr(), _name.get() + "." + name, _level, _callback);
    } else {
      res = std::make_shared<Logger>(getptr(), name, _level, _callback);
    }
    res->setVerbosity(getVerbosity());
    return res;
  }
  std::shared_ptr<Logger> Logger::create(EntryLogger callback)
  {
    return std::make_shared<Logger>(callback);
  }
  std::shared_ptr<Logger> Logger::create(EntryLogger callback, const std::string& name)
  {
    return std::make_shared<Logger>(callback, name);
  }

  size_t Logger::getVerbosity() const
  {
    return _verbosity;
  }

  void Logger::setVerbosity(size_t verbosity)
  {
    _verbosity = verbosity;
  }

  Logger::Logger(EntryLogger callback) : _callback(callback)
  {
  }
  Logger::Logger(EntryLogger callback, boost::optional<const std::string> name) : _callback(callback), _name(name)
  {
  }
  Logger::Logger(std::shared_ptr<Logger> parent, boost::optional<const std::string> name, size_t lvl,  EntryLogger callback) : _parent(parent), _callback(callback), _name(name), _level(lvl)
  {
  }
};

std::shared_ptr<Logging::Logger> g_slog{nullptr};
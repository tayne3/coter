/*
 * MIT License
 *
 * Copyright (c) 2026 tayne3
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * 2. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */
#ifndef COTER_INI_INI_H
#define COTER_INI_INI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_ini_s         ct_ini_t;
typedef struct ct_ini_section_s ct_ini_section_t;
typedef struct ct_ini_key_s     ct_ini_key_t;

// -----------------------------------------------------------------------------
// Lifecycle Management
// -----------------------------------------------------------------------------

/**
 * @brief Create a ini object from file
 * @param path INI file path, or NULL to create empty
 * @return ini object, or NULL on failure
 */
CT_API ct_ini_t* ct_ini_create(const char* path);

/**
 * @brief Create an empty ini object
 * @return ini object, or NULL on failure
 */
CT_API ct_ini_t* ct_ini_empty(void);

/**
 * @brief Destroy the ini object and free all memory
 * @param self ini object
 */
CT_API void ct_ini_destroy(ct_ini_t* self);

/**
 * @brief Clear all sections and keys
 * @param self ini object
 */
CT_API void ct_ini_clear(ct_ini_t* self);

/**
 * @brief Load configuration from file
 * @param self ini object
 * @param path INI file path
 * @return 0 on success, error code otherwise
 * @note Merges with existing data; use ct_ini_clear() first if replacement needed
 */
CT_API int ct_ini_load(ct_ini_t* self, const char* path);

/**
 * @brief Set the key/value delimiter used during parsing
 * @param self ini object
 * @param delim delimiter character, currently '=' or ':'
 * @note Invalid delimiters are ignored and the current setting is kept
 */
CT_API void ct_ini_set_delim(ct_ini_t* self, char delim);

/**
 * @brief Enable or disable no-section keys
 * @param self ini object
 * @param enabled true to store no-section keys in the "" section
 */
CT_API void ct_ini_set_nosection(ct_ini_t* self, bool enabled);

/**
 * @brief Save configuration to file
 * @param self ini object
 * @param path output file path
 * @return 0 on success, error code otherwise
 */
CT_API int ct_ini_save_to(ct_ini_t* self, const char* path);

// -----------------------------------------------------------------------------
// Error Handling
// -----------------------------------------------------------------------------

/**
 * @brief Get the last error code
 * @param self ini object
 * @return error code, or -1 if self is NULL
 */
CT_API int ct_ini_last_error(const ct_ini_t* self);

/**
 * @brief Get error description string
 * @param code error code
 * @return error description
 */
CT_API const char* ct_ini_error_string(int code);

// -----------------------------------------------------------------------------
// Section Operations
// -----------------------------------------------------------------------------

/**
 * @brief Get section by name, creating if not exists
 * @param self ini object
 * @param name section name
 * @return section, or NULL on failure
 */
CT_API ct_ini_section_t* ct_ini_get_section(ct_ini_t* self, const char* name);

/**
 * @brief Find section by name
 * @param self ini object
 * @param name section name
 * @return section, or NULL if not found
 */
CT_API ct_ini_section_t* ct_ini_find_section(ct_ini_t* self, const char* name);

/**
 * @brief Remove section by name
 * @param self ini object
 * @param name section name
 * @return 0 on success, error code otherwise
 */
CT_API int ct_ini_remove_section(ct_ini_t* self, const char* name);

/**
 * @brief Get first section for iteration
 * @param self ini object
 * @return first section, or NULL if empty
 */
CT_API ct_ini_section_t* ct_ini_first_section(const ct_ini_t* self);

/**
 * @brief Get next section in iteration
 * @param section current section
 * @return next section, or NULL if at end
 */
CT_API ct_ini_section_t* ct_ini_section_next(const ct_ini_section_t* section);

/**
 * @brief Get section name
 * @param section section object
 * @return section name (owned by section), or NULL if invalid
 */
CT_API const char* ct_ini_section_name(const ct_ini_section_t* section);

// -----------------------------------------------------------------------------
// Key Operations
// -----------------------------------------------------------------------------

/**
 * @brief Get key in section, creating if not exists
 * @param section section object
 * @param key key name
 * @return key, or NULL on failure
 */
CT_API ct_ini_key_t* ct_ini_section_get_key(ct_ini_section_t* section, const char* key);

/**
 * @brief Find key in section
 * @param section section object
 * @param key key name
 * @return key, or NULL if not found
 */
CT_API ct_ini_key_t* ct_ini_section_find_key(const ct_ini_section_t* section, const char* key);

/**
 * @brief Check if key exists in section
 * @param section section object
 * @param key key name
 * @return true if exists
 */
CT_API bool ct_ini_section_has_key(const ct_ini_section_t* section, const char* key);

/**
 * @brief Add or update key with value
 * @param section section object
 * @param key key name
 * @param value string value
 * @return key, or NULL on failure
 */
CT_API ct_ini_key_t* ct_ini_section_add_key(ct_ini_section_t* section, const char* key, const char* value);

/**
 * @brief Remove key from section
 * @param section section object
 * @param key key name
 * @return 0 on success, error code otherwise
 */
CT_API int ct_ini_section_remove_key(ct_ini_section_t* section, const char* key);

/**
 * @brief Get first key in section for iteration
 * @param section section object
 * @return first key, or NULL if empty
 */
CT_API ct_ini_key_t* ct_ini_section_first_key(const ct_ini_section_t* section);

/**
 * @brief Get next key in iteration
 * @param key current key
 * @return next key, or NULL if at end
 */
CT_API ct_ini_key_t* ct_ini_key_next(const ct_ini_key_t* key);

/**
 * @brief Get key name
 * @param key key object
 * @return key name (owned by key), or NULL if invalid
 */
CT_API const char* ct_ini_key_name(const ct_ini_key_t* key);

// -----------------------------------------------------------------------------
// Convenience Functions
// -----------------------------------------------------------------------------

/**
 * @brief Get key by section and key name, creating both if not exist
 * @param self ini object
 * @param section section name (NULL or "" for default)
 * @param key key name
 * @return key, or NULL on failure
 */
CT_API ct_ini_key_t* ct_ini_get_key(ct_ini_t* self, const char* section, const char* key);

/**
 * @brief Find key by section and key name
 * @param self ini object
 * @param section section name (NULL or "" for default)
 * @param key key name
 * @return key, or NULL if not found
 */
CT_API ct_ini_key_t* ct_ini_find_key(ct_ini_t* self, const char* section, const char* key);

// -----------------------------------------------------------------------------
// Value Access & Conversion
// -----------------------------------------------------------------------------

/**
 * @brief Set raw string value
 * @param key key object
 * @param value new value
 */
CT_API void ct_ini_key_set_value(ct_ini_key_t* key, const char* value);

/**
 * @brief Get raw string value
 * @param key key object
 * @return value (owned by key), or "" if invalid
 */
CT_API const char* ct_ini_key_get_value(const ct_ini_key_t* key);

/**
 * @brief Get string value with default
 * @param key key object (may be NULL)
 * @param default_value fallback value
 * @return value or default_value
 */
CT_API const char* ct_ini_key_get(const ct_ini_key_t* key, const char* default_value);

/**
 * @brief Get non-empty string value with default
 * @param key key object
 * @param default_value fallback if NULL, empty, or whitespace-only
 * @return value or default_value
 */
CT_API const char* ct_ini_key_get_string(const ct_ini_key_t* key, const char* default_value);

/**
 * @brief Parse value as int
 * @param key key object
 * @param default_value fallback on parse failure
 * @return parsed value or default_value
 */
CT_API int ct_ini_key_get_int(const ct_ini_key_t* key, int default_value);

/**
 * @brief Parse value as int64_t
 * @param key key object
 * @param default_value fallback on parse failure
 * @return parsed value or default_value
 * @note Supports decimal, hex (0x), octal (0) prefixes
 */
CT_API int64_t ct_ini_key_get_i64(const ct_ini_key_t* key, int64_t default_value);

/**
 * @brief Parse value as uint64_t
 * @param key key object
 * @param default_value fallback on parse failure or negative input
 * @return parsed value or default_value
 * @note Rejects negative values
 */
CT_API uint64_t ct_ini_key_get_u64(const ct_ini_key_t* key, uint64_t default_value);

/**
 * @brief Parse value as double
 * @param key key object
 * @param default_value fallback on parse failure
 * @return parsed value or default_value
 */
CT_API double ct_ini_key_get_double(const ct_ini_key_t* key, double default_value);

/**
 * @brief Parse value as bool
 * @param key key object
 * @param default_value fallback on parse failure
 * @return parsed value or default_value
 * @note Accepts: 1/0, Y/N, y/n, T/F, t/f
 */
CT_API bool ct_ini_key_get_bool(const ct_ini_key_t* key, bool default_value);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace coter {
namespace ini {

    class Key {
    public:
        explicit Key(ct_ini_key_t* k) : d(k) {}
        bool        valid() const { return d != nullptr; }
        explicit    operator bool() const { return valid(); }
        std::string name() const { return ct_ini_key_name(d); }
        Key         next() const { return Key(ct_ini_key_next(d)); }

        std::string   get(const std::string& def = "") const { return ct_ini_key_get(d, def.c_str()); }
        std::string   getValue() const { return ct_ini_key_get_value(d); }
        std::string   getString(const std::string& def = "") const { return ct_ini_key_get_string(d, def.c_str()); }
        int           getInt(int def = 0) const { return ct_ini_key_get_int(d, def); }
        long          getInteger(long def = 0) const { return static_cast<long>(ct_ini_key_get_i64(d, def)); }
        unsigned long getUnsigned(unsigned long def = 0) const {
            return static_cast<unsigned long>(ct_ini_key_get_u64(d, def));
        }
        int64_t  getI64(int64_t def = 0) const { return ct_ini_key_get_i64(d, def); }
        uint64_t getU64(uint64_t def = 0) const { return ct_ini_key_get_u64(d, def); }
        double   getDouble(double def = 0.0) const { return ct_ini_key_get_double(d, def); }
        bool     getBool(bool def = false) const { return ct_ini_key_get_bool(d, def); }

        void set(const std::string& value) { ct_ini_key_set_value(d, value.c_str()); }
        void setValue(const std::string& value) { set(value); }
        void setValue(const char* value) { set(value); }
        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, void>::type setValue(T value) {
            set(std::to_string(value));
        }

    private:
        ct_ini_key_t* d;
    };

    class Section {
    public:
        explicit Section(ct_ini_section_t* s) : d(s) {}
        bool        valid() const { return d != nullptr; }
        explicit    operator bool() const { return valid(); }
        std::string name() const { return ct_ini_section_name(d); }
        Key         firstKey() const { return Key(ct_ini_section_first_key(d)); }
        Section     next() const { return Section(ct_ini_section_next(d)); }

        Key  getKey(const std::string& key) { return Key(ct_ini_section_get_key(d, key.c_str())); }
        Key  findKey(const std::string& key) const { return Key(ct_ini_section_find_key(d, key.c_str())); }
        bool hasKey(const std::string& key) const { return ct_ini_section_has_key(d, key.c_str()); }
        Key  addKey(const std::string& key, const std::string& value = "") {
            return Key(ct_ini_section_add_key(d, key.c_str(), value.c_str()));
        }
        int removeKey(const std::string& key) { return ct_ini_section_remove_key(d, key.c_str()); }

        std::vector<std::string> keys() const {
            std::vector<std::string> result;
            for (Key k = firstKey(); k; k = k.next()) { result.emplace_back(k.name()); }
            return result;
        }

    private:
        ct_ini_section_t* d;
    };

    class Ini {
    public:
        Ini() : d(ct_ini_empty()) {
            if (!d) { throw std::bad_alloc(); }
        }
        ~Ini() { ct_ini_destroy(d); }

        Ini(Ini&& o) noexcept : d(o.d) { o.d = nullptr; }
        Ini& operator=(Ini&& o) noexcept {
            if (this != &o) {
                ct_ini_destroy(d);
                d   = o.d;
                o.d = nullptr;
            }
            return *this;
        }
        Ini(const Ini&)            = delete;
        Ini& operator=(const Ini&) = delete;

        int                lastError() const { return ct_ini_last_error(d); }
        static const char* errorString(int code) { return ct_ini_error_string(code); }

        void setDelim(char delim) { ct_ini_set_delim(d, delim); }
        void setNoSection(bool enabled) { ct_ini_set_nosection(d, enabled); }

        int  load(const std::string& path) { return ct_ini_load(d, path.c_str()); }
        void clear() { ct_ini_clear(d); }
        int  saveTo(const std::string& path) { return ct_ini_save_to(d, path.c_str()); }

        Section getSection(const std::string& name) { return Section(ct_ini_get_section(d, name.c_str())); }
        Section findSection(const std::string& name) const { return Section(ct_ini_find_section(d, name.c_str())); }
        int     removeSection(const std::string& name) { return ct_ini_remove_section(d, name.c_str()); }

        Key getKey(const std::string& section, const std::string& key) {
            return Key(ct_ini_get_key(d, section.c_str(), key.c_str()));
        }
        Key findKey(const std::string& section, const std::string& key) const {
            return Key(ct_ini_find_key(d, section.c_str(), key.c_str()));
        }

        Section firstSection() const { return Section(ct_ini_first_section(d)); }

        std::vector<std::string> sections() const {
            std::vector<std::string> result;
            for (Section s = firstSection(); s; s = s.next()) { result.emplace_back(s.name()); }
            return result;
        }

    private:
        ct_ini_t* d;
    };

}  // namespace ini
}  // namespace coter

#endif

#endif  // COTER_INI_INI_H

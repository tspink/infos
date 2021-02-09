/* SPDX-License-Identifier: MIT */

/*
 * include/util/string.h
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>
#include <infos/util/list.h>

namespace infos {
    namespace util {
        extern size_t strlen(const char *str);
        extern int strncmp(const char *str1, const char *str2, size_t n);
        extern char *strncpy(char *dst, const char *src, size_t n);

        extern "C" void *memcpy(void *dest, const void *src, size_t n);
        extern "C" void *memset(void *dest, int c, size_t n);

        extern "C" void *bzero(void *dest, size_t n);
        extern "C" void pzero(void *dest);
        extern "C" void pnzero(void *dest, size_t n);

        class String {
        public:
            typedef uint64_t hash_type;

            String() : _size(0), _data(new char[1]), _has_hash(false), _hash(0) {
                _data[0] = 0;
            }

            // From const char * constructor

            String(const char *str) : _size(strlen(str)), _data(NULL), _has_hash(false), _hash(0) {
                _data = new char[_size + 1];

                for (unsigned int i = 0; i < _size; i++) {
                    _data[i] = str[i];
                }

                _data[_size] = 0;
            }

            // Copy Constructor

            String(const String& str)
            : _size(str._size),
            _data(NULL),
            _has_hash(str._has_hash),
            _hash(str._hash) {
                _data = new char[_size + 1];

                for (unsigned int i = 0; i < _size; i++) {
                    _data[i] = str._data[i];
                }

                _data[_size] = 0;
            }

            // Move Constructor

            String(String&& str)
            : _size(str._size),
            _data(str._data),
            _has_hash(str._has_hash),
            _hash(str._has_hash) {

            }

            ~String() {
                delete _data;
            }

            /**
             * Returns the length of the string.
             * @return Returns the length of the string.
             */
            inline size_t length() const {
                return _size;
            }

            /**
             * Returns whether or not the string is empty (i.e. length == 0)
             */
            inline bool empty() const {
                return _size == 0;
            }

            /**
             * Returns the C-representation of the string.
             * @return Returns the C-representation of the string.
             */
            const char *c_str() const {
                return _data;
            }

            /**
             * Retrieves the hash of this string, lazily computing it
             * if necessary.
             *
             * @return Returns a 64-bit hash of the string.
             */
            hash_type get_hash() const {
                if (_has_hash) return _hash;

                _hash = compute_hash();
                _has_hash = true;

                return _hash;
            }

            List<String> split(char delim, bool remove_empty);

            friend String operator+(const String& l, const String& r) {
                String n(l.length() + r.length());

                for (unsigned int i = 0; i < l.length(); i++) {
                    n._data[i] = l._data[i];
                }

                unsigned int off = l.length();
                for (unsigned int i = 0; i < r.length(); i++) {
                    n._data[i + off] = r._data[i];
                }

                n._data[n._size] = 0;

                return n;
            }

            friend String operator+(const String& l, const char& r) {
                String n(l._size + 1);

                for (unsigned int i = 0; i < l._size; i++) {
                    n._data[i] = l._data[i];
                }

                n._data[n._size - 1] = r;
                n._data[n._size] = 0;

                return n;
            }

            String& operator=(const String& s) {
                if (this != &s) {
                    delete _data;

                    _size = s._size;
                    _data = new char[_size + 1];
                    for (unsigned int i = 0; i < _size; i++) {
                        _data[i] = s._data[i];
                    }

                    _data[_size] = 0;
                    _has_hash = false;
                }

                return *this;
            }

            String& operator=(String&& s) {
                if (this != &s) {
                    delete _data;

                    _size = s._size;
                    _data = s._data;
                    _has_hash = s._has_hash;
                    _hash = s._hash;

                    s._data = NULL;
                    s._size = 0;
                }

                return *this;
            }

            char operator[](unsigned int idx) const {
                if (idx >= _size) return 0;
                return _data[idx];
            }

            friend bool operator==(const String& l, const String& r) {
                if (l._size != r._size) return false;

                for (unsigned int i = 0; i < l._size; i++) {
                    if (l._data[i] != r._data[i]) return false;
                }

                return true;
            }

        private:

            String(unsigned int new_size)
            : _size(new_size),
            _data(new char[new_size + 1]),
            _has_hash(false),
            _hash(0) {
                _data[_size] = 0;
            }

            /*
             * Computes the FNV-1a hash
             */
            hash_type compute_hash() const {
                // Offset basis for 64-bit hash
                uint64_t hash = 14695981039346656037ULL;

                for (unsigned int i = 0; i < _size; i++) {
                    hash ^= _data[i];

                    // FNV Prime for 64-bit hash
                    hash *= 1099511628211ULL;
                }

                return (hash_type) hash;
            }

            size_t _size;
            char *_data;

            mutable bool _has_hash;
            mutable hash_type _hash;
        };

        extern String ToString(unsigned int v);
    }
}

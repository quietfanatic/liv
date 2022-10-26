#pragma once

#include "common.h"

 // Intrusive double-linked list class.

namespace uni {

template <class T, int id = 0>
struct Links {
    Links<T, id>* prev;
    Links<T, id>* next;
    Links () : prev(this), next(this) { }
    ~Links () { prev->next = next; next->prev = prev; }
    Links (Links&& o) : prev(o.prev), next(o.next) {
        o.prev = &o; o.next = &o;
    }

    void unlink () {
        prev->next = next; next->prev = prev;
        prev = this; next = this;
    }
    void link_after (Links* o) {
        prev->next = next; next->prev = prev;
        prev = o; next = o->next;
        o->next->prev = this; o->next = this;
    }
    void link_before (Links* o) {
        prev->next = next; next->prev = prev;
        prev = o->prev; next = o;
        o->prev->next = this; o->prev = this;
    }
};

template <class T, int id = 0>
struct Linked : Links<T, id> { };

template <class T, int id = 0>
struct LinkedList : Links<T, id> {
    using Links<T, id>::next;
    using Links<T, id>::prev;
    struct iterator {
        Links<T, id>* p;
        T& operator * () const { return *static_cast<T*>(p); }
        T* operator -> () const { return static_cast<T*>(p); }
        operator T* () const { return static_cast<T*>(p); }
        iterator& operator ++ () { p = p->next; return *this; }
        iterator operator ++ (int) { auto r = *this; p = p->next; return r; }
        iterator& operator -- () { p = p->prev; return *this; }
        iterator operator -- (int) { auto r = *this; p = p->prev; return r; }
        bool operator == (const Links<T, id>& o) const { return p == o.p; }
        bool operator != (const Links<T, id>& o) const { return p != o.p; }
    };
    struct reverse_iterator {
        Links<T, id>* p;
        T& operator * () const { return *static_cast<T*>(p); }
        T* operator -> () const { return static_cast<T*>(p); }
        operator T* () const { return static_cast<T*>(p); }
        iterator& operator ++ () { p = p->prev; return *this; }
        iterator operator ++ (int) { auto r = *this; p = p->prev; return r; }
        iterator& operator -- () { p = p->next; return *this; }
        iterator operator -- (int) { auto r = *this; p = p->next; return r; }
        bool operator == (const Links<T, id>& o) const { return p == o.p; }
        bool operator != (const Links<T, id>& o) const { return p != o.p; }
    };
    T& front () { return *static_cast<T*>(next); }
    T& back () { return *static_cast<T*>(prev); }
    iterator begin () { return {next}; }
    iterator end () { return {this}; }
    iterator rbegin () { return {prev}; }
    iterator rend () { return {this}; }
    CE bool empty () const { return next == this; }
    usize size () { usize r = 0; for (auto& l : *this) r++; return r; }
    void clear () { while (!empty) prev->unlink(); }
    template <class Compare>
    void add_sorted (T* e, Compare comp) {
        for (auto it = rbegin(); it != rend(); it++) {
            if (comp(e, &*it)) {
                e->link_after(it); return;
            }
        }
        e->link_after(this);
    }
    void add_sorted (T* e) { add_sorted(e, [](T& a, T& b){ return a < b; }); }
};

} // namespace uni

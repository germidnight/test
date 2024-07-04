#pragma once
/*
 * В этой задаче нужно убрать наследование, но оставить полиморфизм.
 * Вам нужно сделать структуру Vtable по аналогии с той что хранится для виртуальных методов.
 * Ее нужно хранить первый полем.
 * Например, в IdentityDocument первым полем хранить указатель на vtable IdentityDocument::Vtable* vptr_.
 * C помощью метода SetVTable можно устанавливать vtable другому классу.
 */
#include <iostream>
#include <string>

using namespace std::string_view_literals;

class IdentityDocument {
public:
    IdentityDocument()
        : unique_id_(++unique_id_count_)
    {
        IdentityDocument::SetVTable(this);
        std::cout << "IdentityDocument::Ctor() : "sv << unique_id_ << std::endl;
    }

    ~IdentityDocument() {
        --unique_id_count_;
        std::cout << "IdentityDocument::Dtor() : "sv << unique_id_ << std::endl;
    }

    IdentityDocument(const IdentityDocument&)
        : unique_id_(++unique_id_count_)
    {
        IdentityDocument::SetVTable(this);
        std::cout << "IdentityDocument::CCtor() : "sv << unique_id_ << std::endl;
    }

    IdentityDocument& operator=(const IdentityDocument&) = delete;

    static void PrintUniqueIDCount() {
        std::cout << "unique_id_count_ : "sv << unique_id_count_ << std::endl;
    }

    using DeleteFunction = void(*)(IdentityDocument*);
    using PrintFunction = void(*)(const IdentityDocument*);

    struct Vtable {
        DeleteFunction delete_this;
        PrintFunction print_this;
    };

    static void SetVTable(IdentityDocument* obj) {
        *(IdentityDocument::Vtable**) obj = &IdentityDocument::VTABLE;
    }

    void PrintID() const {
        GetVtable()->print_this(this);
    }

    void Delete() {
        GetVtable()->delete_this(this);
    }

    const IdentityDocument::Vtable* GetVtable() const {
        return vptr_;
    }

    IdentityDocument::Vtable* GetVtable() {
        return vptr_;
    }

    int GetID() const {
        return unique_id_;
    }

private:
    IdentityDocument::Vtable* vptr_ = nullptr;
    static IdentityDocument::Vtable VTABLE;

    static void Delete(IdentityDocument* obj) {
        /* В этот момент тип объекта известен. Просто удаляем указатель.
         * Вызов delete запустит процесс вызовов деструкторов*/
        delete obj;
    }

    static void PrintID(const IdentityDocument* obj) {
        std::cout << "IdentityDocument::PrintID() : "sv << obj->unique_id_ << std::endl;
    }

    static int unique_id_count_;
    int unique_id_;
};

IdentityDocument::Vtable IdentityDocument::VTABLE = { IdentityDocument::Delete, IdentityDocument::PrintID };

int IdentityDocument::unique_id_count_ = 0;
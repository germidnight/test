#pragma once

#include "identity_document.h"
#include <iostream>
#include <string>

using namespace std::string_view_literals;

class TravelPack {
public:
    TravelPack()
        : identity_doc1_(new Passport())
        , identity_doc2_(new DrivingLicence())
    {
        TravelPack::SetVTable(this);
        std::cout << "TravelPack::Ctor()"sv << std::endl;
    }

    TravelPack(const TravelPack& other)
        : identity_document_(other.identity_document_)
        , identity_doc1_(new Passport(*dynamic_cast<Passport*>(other.identity_doc1_)))
        , identity_doc2_(new DrivingLicence(*dynamic_cast<DrivingLicence*>(other.identity_doc2_)))
        , additional_pass_(other.additional_pass_)
        , additional_dr_licence_(other.additional_dr_licence_)
    {
        TravelPack::SetVTable(this);
        std::cout << "TravelPack::CCtor()"sv << std::endl;
    }

    ~TravelPack() {
        //delete identity_doc1_;
        delete identity_doc2_;
        std::cout << "TravelPack::Dtor()"sv << std::endl;
        IdentityDocument::SetVTable((IdentityDocument*)this);
    }

    static void PrintUniqueIDCount() {
        IdentityDocument::PrintUniqueIDCount();
    }

    using DeleteFunction = void (*)(TravelPack *);
    using PrintFunction = void (*)(const TravelPack *);

    struct Vtable {
        DeleteFunction delete_this;
        PrintFunction print_this;
    };

    static void SetVTable(TravelPack *obj) {
        *(TravelPack::Vtable **)obj = &TravelPack::VTABLE;
    }

    void PrintID() const {
        GetVtable()->print_this(this);
    }

    void Delete() {
        GetVtable()->delete_this(this);
    }

    operator IdentityDocument() {
        return {identity_document_};
    }

    const Vtable *GetVtable() const {
        return (const TravelPack::Vtable *)identity_document_.GetVtable();
    }

    Vtable *GetVtable() {
        return (TravelPack::Vtable *)identity_document_.GetVtable();
    }

    int GetID() const {
        return identity_document_.GetID();
    }

private:
    IdentityDocument identity_document_;

    static TravelPack::Vtable VTABLE;

    static void Delete(TravelPack *obj) {
        delete obj;
    }

    static void PrintID(const TravelPack *obj) {
        obj->identity_doc1_->PrintID();
        obj->identity_doc2_->PrintID();
        obj->additional_pass_.PrintID();
        obj->additional_dr_licence_.PrintID();
    }

    Passport* identity_doc1_;
    DrivingLicence* identity_doc2_;
    Passport additional_pass_;
    DrivingLicence additional_dr_licence_;
};

TravelPack::Vtable TravelPack::VTABLE = {TravelPack::Delete, TravelPack::PrintID};
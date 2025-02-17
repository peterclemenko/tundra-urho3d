// For conditions of distribution and use, see copyright notice in LICENSE
// This file has been autogenerated with BindingsGenerator

#include "StableHeaders.h"
#include "CoreTypes.h"
#include "BindingsHelpers.h"
#include "Framework/FrameAPI.h"

#ifdef _MSC_VER
#pragma warning(disable: 4800)
#endif

#include "Framework/Framework.h"


using namespace Tundra;
using namespace std;

namespace JSBindings
{



const char* FrameAPI_ID = "FrameAPI";

const char* SignalWrapper_FrameAPI_Updated_ID = "SignalWrapper_FrameAPI_Updated";

class SignalWrapper_FrameAPI_Updated
{
public:
    SignalWrapper_FrameAPI_Updated(Urho3D::Object* owner, Signal1< float >* signal) :
        owner_(owner),
        signal_(signal)
    {
    }

    Urho3D::WeakPtr<Urho3D::Object> owner_;
    Signal1< float >* signal_;
};

class SignalReceiver_FrameAPI_Updated : public SignalReceiver
{
public:
    void ForwardSignal(float param0)
    {
        duk_context* ctx = ctx_;
        duk_push_global_object(ctx);
        duk_get_prop_string(ctx, -1, "_DispatchSignal");
        duk_push_number(ctx, param0);
        duk_pcall(ctx, 1);
        duk_pop(ctx);
        duk_pop(ctx);
    }
};

duk_ret_t SignalWrapper_FrameAPI_Updated_Finalizer(duk_context* ctx)
{
    SignalWrapper_FrameAPI_Updated* obj = GetValueObject<SignalWrapper_FrameAPI_Updated>(ctx, 0, SignalWrapper_FrameAPI_Updated_ID);
    if (obj)
    {
        delete obj;
        SetValueObject(ctx, 0, 0, SignalWrapper_FrameAPI_Updated_ID);
    }
    return 0;
}

static duk_ret_t SignalWrapper_FrameAPI_Updated_Emit(duk_context* ctx)
{
    SignalWrapper_FrameAPI_Updated* wrapper = GetThisValueObject<SignalWrapper_FrameAPI_Updated>(ctx, SignalWrapper_FrameAPI_Updated_ID);
    if (!wrapper->owner_) return 0; // Check signal owner expiration
    float param0 = (float)duk_require_number(ctx, 0);
    wrapper->signal_->Emit(param0);
    return 0;
}

static duk_ret_t FrameAPI_Get_Updated(duk_context* ctx)
{
    FrameAPI* thisObj = GetThisWeakObject<FrameAPI>(ctx);
    SignalWrapper_FrameAPI_Updated* wrapper = new SignalWrapper_FrameAPI_Updated(thisObj, &thisObj->Updated);
    PushValueObject(ctx, wrapper, SignalWrapper_FrameAPI_Updated_ID, SignalWrapper_FrameAPI_Updated_Finalizer, false);
    duk_push_c_function(ctx, SignalWrapper_FrameAPI_Updated_Emit, 1);
    duk_put_prop_string(ctx, -2, "Emit");
    return 1;
}

const char* SignalWrapper_FrameAPI_PostFrameUpdate_ID = "SignalWrapper_FrameAPI_PostFrameUpdate";

class SignalWrapper_FrameAPI_PostFrameUpdate
{
public:
    SignalWrapper_FrameAPI_PostFrameUpdate(Urho3D::Object* owner, Signal1< float >* signal) :
        owner_(owner),
        signal_(signal)
    {
    }

    Urho3D::WeakPtr<Urho3D::Object> owner_;
    Signal1< float >* signal_;
};

class SignalReceiver_FrameAPI_PostFrameUpdate : public SignalReceiver
{
public:
    void ForwardSignal(float param0)
    {
        duk_context* ctx = ctx_;
        duk_push_global_object(ctx);
        duk_get_prop_string(ctx, -1, "_DispatchSignal");
        duk_push_number(ctx, param0);
        duk_pcall(ctx, 1);
        duk_pop(ctx);
        duk_pop(ctx);
    }
};

duk_ret_t SignalWrapper_FrameAPI_PostFrameUpdate_Finalizer(duk_context* ctx)
{
    SignalWrapper_FrameAPI_PostFrameUpdate* obj = GetValueObject<SignalWrapper_FrameAPI_PostFrameUpdate>(ctx, 0, SignalWrapper_FrameAPI_PostFrameUpdate_ID);
    if (obj)
    {
        delete obj;
        SetValueObject(ctx, 0, 0, SignalWrapper_FrameAPI_PostFrameUpdate_ID);
    }
    return 0;
}

static duk_ret_t SignalWrapper_FrameAPI_PostFrameUpdate_Emit(duk_context* ctx)
{
    SignalWrapper_FrameAPI_PostFrameUpdate* wrapper = GetThisValueObject<SignalWrapper_FrameAPI_PostFrameUpdate>(ctx, SignalWrapper_FrameAPI_PostFrameUpdate_ID);
    if (!wrapper->owner_) return 0; // Check signal owner expiration
    float param0 = (float)duk_require_number(ctx, 0);
    wrapper->signal_->Emit(param0);
    return 0;
}

static duk_ret_t FrameAPI_Get_PostFrameUpdate(duk_context* ctx)
{
    FrameAPI* thisObj = GetThisWeakObject<FrameAPI>(ctx);
    SignalWrapper_FrameAPI_PostFrameUpdate* wrapper = new SignalWrapper_FrameAPI_PostFrameUpdate(thisObj, &thisObj->PostFrameUpdate);
    PushValueObject(ctx, wrapper, SignalWrapper_FrameAPI_PostFrameUpdate_ID, SignalWrapper_FrameAPI_PostFrameUpdate_Finalizer, false);
    duk_push_c_function(ctx, SignalWrapper_FrameAPI_PostFrameUpdate_Emit, 1);
    duk_put_prop_string(ctx, -2, "Emit");
    return 1;
}

void Expose_FrameAPI(duk_context* ctx)
{
    duk_push_object(ctx);
    duk_push_object(ctx);
    DefineProperty(ctx, "updated", FrameAPI_Get_Updated, nullptr);
    DefineProperty(ctx, "postFrameUpdate", FrameAPI_Get_PostFrameUpdate, nullptr);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, FrameAPI_ID);
}

}

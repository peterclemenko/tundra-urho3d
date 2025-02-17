// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"

#include "SceneContextMenu.h"
#include "Framework.h"

#include "LoggingFunctions.h"

#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Menu.h>

namespace Tundra
{

SceneContextMenu::SceneContextMenu(Context* context) :
    Object(context)
{
    XMLFile *style = context_->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Data/UI/DefaultStyle.xml");

    window_ = new Window(context);
    window_->SetStyle("Window", style);
    window_->SetName("SceneContextMenu");
    window_->SetMinWidth(150);
    window_->SetMinHeight(67);
    window_->SetLayoutMode(LM_VERTICAL);
    window_->SetLayout(LM_VERTICAL, 4, IntRect(6, 6, 6, 6));
    window_->SetMovable(false);
    window_->SetEnabled(true);
    SubscribeToEvent(window_, E_DEFOCUSED, URHO3D_HANDLER(SceneContextMenu, OnWindowDefocused));
    Close();
}

SceneContextMenu::~SceneContextMenu()
{
    if (window_.NotNull())
        window_->Remove();
    window_.Reset();
}

Menu *SceneContextMenu::GetItem(const String &id)
{
    if (contextItemMap_.Contains(id))
        return contextItemMap_[id];
    return NULL;
}

Menu *SceneContextMenu::CreateItem(const String &id, const String &text)
{
    Menu *m = GetItem(id);
    if (m != NULL)
    {
        Text *t = dynamic_cast<Text*>(m->GetChild(0));
        if (t != NULL)
            t->SetText(text);
    }
    else
    {
        XMLFile *style = context_->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Data/UI/DefaultStyle.xml");

        Menu *m = new Menu(context_);
        m->SetStyle("Menu", style);
        m->SetName(id);
        m->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
        m->SetEnabled(true);
        SubscribeToEvent(m, E_MENUSELECTED, URHO3D_HANDLER(SceneContextMenu, OnItemPressed));

        Text *t = new Text(context_);
        t->SetText(text);
        t->SetStyle("Text", style);
        m->AddChild(t);
        window_->AddChild(m);
        contextItemMap_[id] = MenuWeakPtr(m);
    }
    return m;
}

void SceneContextMenu::Clear()
{
    SceneContextItemMap::Iterator iter = contextItemMap_.Begin();
    while (iter != contextItemMap_.End())
    {
        iter->second_->Remove();
        iter->second_.Reset();
        iter++;
    }
    contextItemMap_.Clear();
}

UIElement *SceneContextMenu::Widget()
{
    return window_;
}

bool SceneContextMenu::IsVisible() const
{
    return window_->IsVisible();
}

void SceneContextMenu::Open()
{
    if (window_ == NULL)
        return;

    window_->SetEnabled(true);
    window_->SetVisible(true);
    window_->SetFocus(true);
    window_->BringToFront();
}

void SceneContextMenu::Close()
{
    if (window_ == NULL)
        return;

    window_->SetEnabled(false);
    window_->SetVisible(false);
}

String SceneContextMenu::GetItemId(Menu *menu)
{
    SceneContextItemMap::ConstIterator iter = contextItemMap_.Begin();
    while (iter != contextItemMap_.End())
    {
        if (iter->second_ == menu)
            return iter->first_;
        iter++;
    }
    return "";
}

void SceneContextMenu::OnItemPressed(StringHash /*eventType*/, VariantMap& eventData)
{
    String id = GetItemId(dynamic_cast<Menu*>(eventData["Element"].GetPtr()));
    if (id != "")
        OnActionSelected.Emit(this, id);
    Close();
}

void SceneContextMenu::OnWindowDefocused(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    LogWarning("Defocused!");
}

}
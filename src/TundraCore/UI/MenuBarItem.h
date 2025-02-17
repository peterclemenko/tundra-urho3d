// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "FrameworkFwd.h"
#include "CoreTypes.h"
#include "Signals.h"

#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/UI/UIElement.h>


namespace Urho3D
{
class Menu;
}

using namespace Urho3D;

namespace Tundra
{

class MenuBar;
class MenuBarItem;
typedef SharedPtr<MenuBarItem> MenuBarItemPtr;

/// MenuBarItem
class TUNDRACORE_API MenuBarItem : public Object
{
    URHO3D_OBJECT(MenuBarItem, Object);

public:
    MenuBarItem(const String &title, Framework *framework, MenuBar *menuBar, MenuBarItem *parentItem = 0);
    virtual ~MenuBarItem();

    /// Get MenuBar UiElement
    Menu* GetMenu() const;

    /// Construct a new MenuBarItem and add it as child to this item
    MenuBarItem* CreateMenuItem(const String &title);

    /// Remove child MenuBarItem
    void RemoveMenuItem(const String &title);

    /// Search for MenuBarItem by it's title
    /** Supports both search by title and hierarchical search
        e.g. Find("File") Find("File/Save Scene")
        @param title MenuItem title
        @return MenuBarItem or null if not found
    */
    MenuBarItem* Find(const String &title);

    /// Get child menu item
    MenuBarItem* Child(const String &title);

    /// Get parent MenuBarItem or NULL if no parent.
    MenuBarItem* Parent();

    /// Remove self and children from MenuBar
    void Remove();

    /// Return title text
    String Title() const;

    /// Triggered when MenuItem is pressed
    Signal1<MenuBarItem* ARG(menuitem)> OnItemPressed;

protected:
    /// Create MenuItem object and attach it to parent item.
    void Create(const String &title);

    /// Create popup UiElement
    void CreatePopup();

    /// Release pupup UiElement
    void RemovePopup();

    /// Update popup size to fit content inside.
    void UpdatePopup();

    void OnMenuPressed(StringHash eventType, VariantMap& eventData);

    void UpdateUI(float time);

    Framework *framework_;

private:
    HashMap<String, MenuBarItemPtr> subMenus_;
    SharedPtr<Menu> item_;
    WeakPtr<MenuBar> menuBar_;
    WeakPtr<MenuBarItem> parentItem_;
};

}

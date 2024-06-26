#ifndef _ITEMOBJBANANA_
#define _ITEMOBJBANANA_
#include <kamek.hpp>
#include <MarioKartWii/Item/Obj/ItemObj.hpp>

namespace Item {
class ObjKumo : public Obj { //TC
public:
    static Obj** CreateArray(u32 count); //807af140
    virtual void InitSelf(); //0x8 807af534 vtable 808d2480
    virtual void UpdateModelPosition();  //0x10 807af39c
    virtual void SpawnModel(); //0x18 807af200
    virtual void OnCollision(); //0x24 807b123c
    virtual void vf_0x2c(); //0x2c 807b0960

    void UpdateThunk(); //807a3908
    void Update(); //807af76c

    u8 unknown_0x1a0[0x200 - 0x1a0];
};
size_assert(ObjKumo, 0x200);
}//namespace Item

#endif
#include <Ghost/PULLeaderboard.hpp>
#include <Ghost/GhostManager.hpp>


namespace Pulsar {
namespace Ghosts {
//CTOR to build a leaderboard from scratch
Leaderboard::Leaderboard() {
    memset(this, 0, sizeof(Leaderboard));
    this->magic = Leaderboard::fileMagic;
    this->version = LDBVersion;
    for(int mode = 0; mode < 4; mode++) this->hasTrophy[mode] = false;
}

//CTOR to build it from the raw file
Leaderboard::Leaderboard(const char* folderPath, u32 crc32) {
    char filePath[IOS::ipcMaxPath];
    snprintf(filePath, IOS::ipcMaxPath, "%s/ldb.bin", folderPath);
    IO::File* file = IO::File::sInstance;
    s32 ret = file->Open(filePath, IO::FILE_MODE_READ_WRITE);
    if(ret > 0) ret = file->Read(this, sizeof(Leaderboard));

    if(ret <= 0 || this->crc32 != crc32 || magic != fileMagic) {
        System::sInstance->taskThread->Request(&Leaderboard::CreateFile, crc32, 0);
        new (this) Leaderboard;
        this->crc32 = crc32;
    }
    file->Close();
}

//This is its own function so that the file can be created async 
void Leaderboard::CreateFile(u32 crc32) {
    char filePath[IOS::ipcMaxPath];
    snprintf(filePath, IOS::ipcMaxPath, "%s/ldb.bin", Manager::folderPath);
    IO::File* file = IO::File::sInstance;
    file->CreateAndOpen(filePath, IO::FILE_MODE_READ_WRITE);
    alignas(0x20) Leaderboard tempCopy;
    tempCopy.crc32 = crc32;
    file->Overwrite(sizeof(Leaderboard), &tempCopy);
    file->Close();
};

//Get ldb position
s32 Leaderboard::GetPosition(const Timer& other) const {
    s32 position = -1;
    Timer timer;
    for(int i = ENTRY_10TH; i >= 0; i--) {
        this->EntryToTimer(timer, i);
        if(timer > other) position = i;
    }
    return position;
}

//updates the ldb with a new entry and a rkg crc32
void Leaderboard::Update(u32 position, const TimeEntry& entry, u32 crc32) {
    const TTMode mode = System::sInstance->ttMode;
    if(position != ENTRY_FLAP) { //if 10 then flap
        for(int i = ENTRY_10TH; i > position; i--) memcpy(&this->entries[mode][i], &this->entries[mode][i - 1], sizeof(PULTimeEntry));
        this->entries[mode][position].rkgCRC32 = crc32;
    }
    memcpy(&this->entries[mode][position].mii, &entry.mii, sizeof(RawMii));
    this->entries[mode][position].minutes = entry.timer.minutes;
    this->entries[mode][position].seconds = entry.timer.seconds;
    this->entries[mode][position].milliseconds = entry.timer.milliseconds;
    this->entries[mode][position].isActive = entry.timer.isActive;
    this->entries[mode][position].controllerType = entry.controllerType;
    this->entries[mode][position].character = entry.character;
    this->entries[mode][position].kart = entry.kart;
}

//saves and writes to the file
void Leaderboard::Save(const char* folderPath) {
    char filePath[IOS::ipcMaxPath];
    snprintf(filePath, IOS::ipcMaxPath, "%s/ldb.bin", folderPath);
    IO::File* loader = IO::File::sInstance;
    loader->Open(filePath, IO::FILE_MODE_WRITE);
    loader->Overwrite(sizeof(Leaderboard), this);
    loader->Close();
}

void Leaderboard::EntryToTimer(Timer& dest, u8 id) const {
    TTMode mode = System::sInstance->ttMode;
    dest.minutes = this->entries[mode][id].minutes;
    dest.seconds = this->entries[mode][id].seconds;
    dest.milliseconds = this->entries[mode][id].milliseconds;
    dest.isActive = this->entries[mode][id].isActive;
}

void Leaderboard::EntryToTimeEntry(TimeEntry& dest, u8 id) const {
    this->EntryToTimer(dest.timer, id);
    TTMode mode = System::sInstance->ttMode;
    memcpy(&dest.mii, &this->entries[mode][id].mii, sizeof(RawMii));
    dest.character = this->entries[mode][id].character;
    dest.kart = this->entries[mode][id].kart;
    dest.controllerType = this->entries[mode][id].controllerType;
}

//PULEntry to TimeEntry
const TimeEntry* Leaderboard::GetEntry(u32 index) {
    Manager* manager = Manager::sInstance;
    manager->GetLeaderboard().EntryToTimeEntry(manager->entry, index);
    return &manager->entry;
}
kmWrite32(0x8085d5bc, 0x3860000a);
kmCall(0x8085d5c8, Leaderboard::GetEntry);
kmWrite32(0x8085d784, 0x3860000b);
kmWrite32(0x8085d8c8, 0x5743063E);
kmCall(0x8085d8d0, Leaderboard::GetEntry); //actual leaderboard
//kmWrite32(0x8085d8e8, 0x60000000);
kmWrite32(0x8085da14, 0x281a000a);
kmWrite32(0x8085da2c, 0x83230028);
kmWrite32(0x8085da4c, 0x3860000a);
kmCall(0x8085da54, Leaderboard::GetEntry);
//kmWrite32(0x8085da6c, 0x60000000);

//Correct BMG if you beat the expert
kmWrite32(0x8085d744, 0x38805000);
int Leaderboard::ExpertBMGDisplay() {
    Manager* manager = Manager::sInstance;
    manager->GetLeaderboard().EntryToTimer(manager->entry.timer, ENTRY_1ST);
    const Timer& expert = manager->GetExpert();
    if(expert.isActive && expert > manager->entry.timer) return 2;
    return 1;
}
kmCall(0x8085dc0c, Leaderboard::ExpertBMGDisplay);
kmWrite32(0x8085dc10, 0x38000002);

}//namespace Ghosts
}//namespace Pulsar
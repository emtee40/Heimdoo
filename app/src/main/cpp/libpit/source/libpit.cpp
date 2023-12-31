/* Copyright (c) 2010-2017 Benjamin Dobell, Glass Echidna
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.*/

// libpit
#include "libpit.h"

using namespace libpit;

PitEntry::PitEntry() {
    binaryType = false;
    deviceType = 0;
    identifier = 0;
    attributes = 0;
    updateAttributes = 0;
    blockSizeOrOffset = 0;
    blockCount = 0;
    fileOffset = 0;
    fileSize = 0;

    memset(partitionName, 0, PitEntry::kPartitionNameMaxLength);
    memset(flashFilename, 0, PitEntry::kFlashFilenameMaxLength);
    memset(fotaFilename, 0, PitEntry::kFotaFilenameMaxLength);
}

PitEntry::~PitEntry() {
}

bool PitEntry::Matches(const PitEntry *otherPitEntry) const {
    if (binaryType == otherPitEntry->binaryType && deviceType == otherPitEntry->deviceType &&
        identifier == otherPitEntry->identifier
        && attributes == otherPitEntry->attributes &&
        updateAttributes == otherPitEntry->updateAttributes &&
        blockSizeOrOffset == otherPitEntry->blockSizeOrOffset
        && blockCount == otherPitEntry->blockCount && fileOffset == otherPitEntry->fileOffset &&
        fileSize == otherPitEntry->fileSize
        && strcmp(partitionName, otherPitEntry->partitionName) == 0 &&
        strcmp(flashFilename, otherPitEntry->flashFilename) == 0
        && strcmp(fotaFilename, otherPitEntry->fotaFilename) == 0) {
        return (true);
    } else {
        return (false);
    }
}


PitData::PitData() {
    entryCount = 0;

    com_tar2[0] = '\0';
    cpu_bl_id[0] = '\0';

    luCount = 0;
}

PitData::~PitData() {
    for (unsigned int i = 0; i < entries.size(); i++)
        delete entries[i];
}

bool PitData::Unpack(const unsigned char *data) {
    if (PitData::UnpackInteger(data, 0) != PitData::kFileIdentifier)
        return (false);

    // Remove existing entries
    for (unsigned int i = 0; i < entries.size(); i++)
        delete entries[i];

    entryCount = PitData::UnpackInteger(data, 4);

    entries.resize(entryCount);

    if (!memcpy(com_tar2, &data[8], 8))
        return (false);
    com_tar2[8] = '\0';
    if (!memcpy(cpu_bl_id, &data[16], 8))
        return (false);
    cpu_bl_id[8] = '\0';

    luCount = PitData::UnpackShort(data, 24);

    unsigned int integerValue;
    unsigned int entryOffset;

    for (unsigned int i = 0; i < entryCount; i++) {
        entryOffset = PitData::kHeaderDataSize + i * PitEntry::kDataSize;

        entries[i] = new PitEntry();

        integerValue = PitData::UnpackInteger(data, entryOffset);
        entries[i]->SetBinaryType(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 4);
        entries[i]->SetDeviceType(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 8);
        entries[i]->SetIdentifier(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 12);
        entries[i]->SetAttributes(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 16);
        entries[i]->SetUpdateAttributes(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 20);
        entries[i]->SetBlockSizeOrOffset(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 24);
        entries[i]->SetBlockCount(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 28);
        entries[i]->SetFileOffset(integerValue);

        integerValue = PitData::UnpackInteger(data, entryOffset + 32);
        entries[i]->SetFileSize(integerValue);

        entries[i]->SetPartitionName((const char *) data + entryOffset + 36);
        entries[i]->SetFlashFilename(
                (const char *) data + entryOffset + 36 + PitEntry::kPartitionNameMaxLength);
        entries[i]->SetFotaFilename(
                (const char *) data + entryOffset + 36 + PitEntry::kPartitionNameMaxLength +
                PitEntry::kFlashFilenameMaxLength);
    }
    return (true);
}

void PitData::Pack(unsigned char *data) const {
    PitData::PackInteger(data, 0, PitData::kFileIdentifier);

    PitData::PackInteger(data, 4, entryCount);

    memcpy(&data[8], com_tar2, 8);
    memcpy(&data[16], cpu_bl_id, 8);

    PitData::PackShort(data, 24, luCount);

    int entryOffset;

    for (unsigned int i = 0; i < entryCount; i++) {
        entryOffset = PitData::kHeaderDataSize + i * PitEntry::kDataSize;

        PitData::PackInteger(data, entryOffset, entries[i]->GetBinaryType());

        PitData::PackInteger(data, entryOffset + 4, entries[i]->GetDeviceType());
        PitData::PackInteger(data, entryOffset + 8, entries[i]->GetIdentifier());
        PitData::PackInteger(data, entryOffset + 12, entries[i]->GetAttributes());

        PitData::PackInteger(data, entryOffset + 16, entries[i]->GetUpdateAttributes());

        PitData::PackInteger(data, entryOffset + 20, entries[i]->GetBlockSizeOrOffset());
        PitData::PackInteger(data, entryOffset + 24, entries[i]->GetBlockCount());

        PitData::PackInteger(data, entryOffset + 28, entries[i]->GetFileOffset());
        PitData::PackInteger(data, entryOffset + 32, entries[i]->GetFileSize());

        memcpy(data + entryOffset + 36, entries[i]->GetPartitionName(),
               PitEntry::kPartitionNameMaxLength);
        memcpy(data + entryOffset + 36 + PitEntry::kPartitionNameMaxLength,
               entries[i]->GetFlashFilename(), PitEntry::kFlashFilenameMaxLength);
        memcpy(data + entryOffset + 36 + PitEntry::kPartitionNameMaxLength +
               PitEntry::kFlashFilenameMaxLength,
               entries[i]->GetFotaFilename(), PitEntry::kFotaFilenameMaxLength);
    }
}

bool PitData::Matches(const PitData *otherPitData) const {
    if (entryCount == otherPitData->entryCount &&
        (strncmp(com_tar2, otherPitData->com_tar2, 8) == 0) &&
        (strncmp(cpu_bl_id, otherPitData->cpu_bl_id, 8) == 0) &&
        luCount == otherPitData->luCount) {
        for (unsigned int i = 0; i < entryCount; i++) {
            if (!entries[i]->Matches(otherPitData->entries[i]))
                return (false);
        }
        return (true);
    } else {
        return (false);
    }
}

void PitData::Clear(void) {
    entryCount = 0;

    com_tar2[0] = '\0';

    cpu_bl_id[0] = '\0';

    luCount = 0;

    for (unsigned int i = 0; i < entries.size(); i++)
        delete entries[i];

    entries.clear();
}

PitEntry *PitData::GetEntry(unsigned int index) {
    return (entries[index]);
}

const PitEntry *PitData::GetEntry(unsigned int index) const {
    return (entries[index]);
}

PitEntry *PitData::FindEntry(const char *partitionName) {
    for (unsigned int i = 0; i < entries.size(); i++) {
        if (entries[i]->IsFlashable() && strcmp(entries[i]->GetPartitionName(), partitionName) == 0)
            return (entries[i]);
    }
    return (nullptr);
}

const PitEntry *PitData::FindEntry(const char *partitionName) const {
    for (unsigned int i = 0; i < entries.size(); i++) {
        if (entries[i]->IsFlashable() && strcmp(entries[i]->GetPartitionName(), partitionName) == 0)
            return (entries[i]);
    }
    return (nullptr);
}

PitEntry *PitData::FindEntry(unsigned int partitionIdentifier) {
    for (unsigned int i = 0; i < entries.size(); i++) {
        if (entries[i]->IsFlashable() && entries[i]->GetIdentifier() == partitionIdentifier)
            return (entries[i]);
    }
    return (nullptr);
}

const PitEntry *PitData::FindEntry(unsigned int partitionIdentifier) const {
    for (unsigned int i = 0; i < entries.size(); i++) {
        if (entries[i]->IsFlashable() && entries[i]->GetIdentifier() == partitionIdentifier)
            return (entries[i]);
    }
    return (nullptr);
}

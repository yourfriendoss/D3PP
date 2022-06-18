//
// Created by Wande on 6/15/2022.
//

#ifndef D3PP_CPPNBT_H
#define D3PP_CPPNBT_H





#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include "compression.h"
#include "Utils.h"

namespace Nbt {
    enum CompressionMode {
        NONE = 0,
        DETECT = 1,
        GZip = 2,
        ZLib = 3
    };

    enum TagType {
        TAG_END,
        TAG_BYTE,
        TAG_SHORT,
        TAG_INT,
        TAG_LONG,
        TAG_FLOAT,
        TAG_DOUBLE,
        TAG_BYTE_ARRAY,
        TAG_STRING,
        TAG_LIST,
        TAG_COMPOUND,
        TAG_INT_ARRAY,
        TAG_LONG_ARRAY
    };

    typedef std::nullptr_t TagEnd;


    typedef std::int8_t TagByte;
    typedef std::int16_t TagShort;
    typedef std::int32_t TagInt;
    typedef std::int64_t TagLong;


    typedef float TagFloat;
    typedef double TagDouble;


    typedef std::vector<TagByte> TagByteArray;
    typedef std::vector<TagInt> TagIntArray;
    typedef std::vector<TagLong> TagLongArray;


    typedef std::string TagString;


    struct TagList;
    struct TagCompound;


    typedef std::variant<TagEnd, TagByte, TagShort, TagInt, TagLong, TagFloat,
            TagDouble, TagByteArray, TagString, TagList, TagCompound, TagIntArray,
            TagLongArray>
            Tag;

    struct TagList {
        int size;
        std::variant<std::vector<TagEnd>, std::vector<TagByte>,
                std::vector<TagShort>, std::vector<TagInt>, std::vector<TagLong>,
                std::vector<TagFloat>, std::vector<TagDouble>, std::vector<TagByteArray>,
                std::vector<TagString>, std::vector<TagList>, std::vector<TagCompound>,
                std::vector<TagIntArray>, std::vector<TagLongArray>>
                base;
    };

    struct TagCompound {
        std::string name;
        std::map<std::string, Tag> data;

        Tag& operator[](const std::string& key) {
            return data[key];
        }
        Tag& operator[](const char* key) {
            return data[key];
        }

        Tag& at(const std::string& key) {
            return data.at(key);
        }
        const Tag& at(const std::string& key) const {
            return data.at(key);
        }

        template <typename T> T& at(const std::string& key) {
            return std::get<T>(data.at(key));
        }
    };

    class NbtFile {
    public:
        static bool Load(const std::string &file, CompressionMode compression = CompressionMode::DETECT) {
            if (!std::filesystem::exists(file)) {
                throw std::invalid_argument("Invalid argument 'file', file does not exist.");
            }

            std::ifstream is(file, std::ios::binary);

            if (!is.is_open()) {
                throw std::runtime_error("Failed to open NBT file");
            }

            int fileSize = Utils::FileSize(file);

            std::vector<unsigned char> data;
            data.resize(fileSize);
            data.assign(std::istreambuf_iterator<char>(is),
                        std::istreambuf_iterator<char>());
            is.close();

            if (compression == CompressionMode::DETECT) {
                compression = DetectCompression(data);
            }

            if (compression == CompressionMode::ZLib || compression == CompressionMode::GZip) {
                int allocSize = 67108864;
                data.reserve(allocSize);
                int decompSize = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.data()), allocSize, file);
                data.resize(decompSize);
            }

            Tag result = Decode(data);
            Serialize(result);
        }

        static bool Save(Tag t, const std::string &filename) {

        }

        static std::string Serialize(TagByte t, std::string name) {
            std::stringstream output;
            output << "TAG_Byte(";
            output << name;
            output << "): ";
            output << t << std::endl;
            return output.str();
        }

        static std::string Serialize(TagCompound t) {
            std::stringstream output;
            output << "TAG_Compound(";
            t.name == "" ? output << "None" : output << "'" << t.name << "'";
            output << "): {" << std::endl;
                for(auto const &p: t.data) {
                    output << "\t" << Serialize(p.second, p.first);
                }
            output << "}";
            return output.str();
        }

    protected:
        static Tag Decode(std::vector<unsigned char> data) {
            if (data.at(0) != TAG_COMPOUND) {
                throw std::runtime_error("TAG_COMPOUND is not the base");
            }

            TagCompound base;
            int offset = 0;
            base.name = ReadString(data, offset);

            ReadCompound(data, offset);
        }

        static TagCompound ReadCompound(std::vector<unsigned char> data, int offset) {
            TagType nextType = TAG_END;
            TagCompound baseTag;
            std::string nextName = "";
            do {
                nextType = static_cast<TagType>(data.at(offset++));
                nextName = ReadString(data, offset);
                Tag nextTag = ReadOnType(data, offset, nextType);

                baseTag.data.insert(std::make_pair(nextName, nextTag));
            } while (nextType != TAG_END);

            return baseTag;
        }

        static Tag ReadOnType(std::vector<unsigned char> data, int& offset, TagType nextType) {
            Tag nextTag;
            switch (nextType) {
                case TAG_BYTE: {
                    auto val = ReadByte(data, offset);
                    nextTag = val;
                    break;
                    }
                case TAG_SHORT: {
                    auto val2 = ReadShort(data, offset);
                    nextTag = val2;
                    break;
                }
                case TAG_INT: {
                    auto val3 = ReadInt(data, offset);
                    nextTag = val3;
                    break;
                }
                case TAG_LONG: {
                    auto val4 = ReadLong(data, offset);
                    nextTag = val4;
                    break;
                }
                case TAG_FLOAT: {
                    auto va5l = ReadFloat(data, offset);
                    nextTag = va5l;
                    break;
                }
                case TAG_DOUBLE: {
                    auto va6l = ReadDouble(data, offset);
                    nextTag = va6l;
                    break;
                }
                case TAG_BYTE_ARRAY: {
                    auto val7 = ReadByteArray(data, offset);
                    nextTag = val7;
                    break;
                }
                case TAG_STRING: {
                    TagString val8 = ReadString(data, offset);
                    nextTag = val8;
                    break;
                }
                case TAG_LIST: {
                    TagList val9 = ReadList(data, offset);
                    nextTag = val9;
                    break;
                }
                case TAG_COMPOUND: {
                    TagCompound val99 = ReadCompound(data, offset);
                    nextTag = val99;
                    break;
                }
                case TAG_INT_ARRAY: {
                    TagIntArray val88 = ReadIntArray(data, offset);
                    nextTag = val88;
                    break;
                }
                case TAG_LONG_ARRAY: {
                    TagLongArray val77 = ReadLongArray(data, offset);
                    nextTag = val77;
                    break;
                }
            }

            return nextTag;
        }

        static TagList ReadList(std::vector<unsigned char> data, int& offset) {
            TagType listType = static_cast<TagType>(data.at(offset++));
            TagInt listLength = ReadInt(data, offset);
            TagList result;
            if (listLength <= 0)
                return result;

            for(auto i = 0; i < listLength; i++) {
            switch (listType) {
                case TAG_BYTE: {
                    std::vector<TagByte> vec;
                    vec.push_back(std::get<TagByte>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_SHORT:{
                    std::vector<TagShort> vec;
                    vec.push_back(std::get<TagShort>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_INT:{
                    std::vector<TagInt> vec;
                    vec.push_back(std::get<TagInt>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_LONG:{
                    std::vector<TagLong> vec;
                    vec.push_back(std::get<TagLong>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_FLOAT:{
                    std::vector<TagFloat> vec;
                    vec.push_back(std::get<TagFloat>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_DOUBLE:{
                    std::vector<TagDouble> vec;
                    vec.push_back(std::get<TagDouble>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_BYTE_ARRAY:{
                    std::vector<TagByteArray> vec;
                    vec.push_back(std::get<TagByteArray>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_STRING:{
                    std::vector<TagString> vec;
                    vec.push_back(std::get<TagString>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_LIST:{
                    std::vector<TagList> vec;
                    vec.push_back(std::get<TagList>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_COMPOUND:{
                    std::vector<TagCompound> vec;
                    vec.push_back(std::get<TagCompound>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_INT_ARRAY:{
                    std::vector<TagIntArray> vec;
                    vec.push_back(std::get<TagIntArray>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_LONG_ARRAY:{
                    std::vector<TagLongArray> vec;
                    vec.push_back(std::get<TagLongArray>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
            }


            }

            return result;
        }
        static TagByte ReadByte(std::vector<unsigned char> data, int& offset) {
            return data.at(offset++);
        }

        static TagShort ReadShort(std::vector<unsigned char> data, int& offset) {
            short val = 0;
            val |= data.at(offset++) << 8;
            val |= data.at(offset++);
            return val;
        }

        static TagInt ReadInt(std::vector<unsigned char> data, int& offset) {
            int result = 0;
            result |= data.at(offset++) << 24;
            result |= data.at(offset++) << 16;
            result |= data.at(offset++) << 8;
            result |= data.at(offset++);
            return result;
        }

        static TagLong ReadLong(std::vector<unsigned char> data, int& offset) {
            TagLong result = 0;
            result |= data.at(offset++) << 56;
            result |= data.at(offset++) << 48;
            result |= data.at(offset++) << 40;
            result |= data.at(offset++) << 32;
            result |= data.at(offset++) << 24;
            result |= data.at(offset++) << 16;
            result |= data.at(offset++) << 8;
            result |= data.at(offset++);
            return result;
        }

        static TagFloat ReadFloat(std::vector<unsigned char> data, int& offset) {
            int result = 0;
            result |= data.at(offset++) << 24;
            result |= data.at(offset++) << 16;
            result |= data.at(offset++) << 8;
            result |= data.at(offset++);

            return static_cast<TagFloat>(result);
        }

        static TagDouble ReadDouble(std::vector<unsigned char> data, int& offset) {
            long result = 0;
            result |= (long)data.at(offset++) << 56;
            result |= (long)data.at(offset++) << 48;
            result |= (long)data.at(offset++) << 40;
            result |= (long)data.at(offset++) << 32;
            result |= (long)data.at(offset++) << 24;
            result |= (long)data.at(offset++) << 16;
            result |= (long)data.at(offset++) << 8;
            result |= (long)data.at(offset++);
            return static_cast<TagDouble>(result);
        }

        static TagByteArray ReadByteArray(std::vector<unsigned char> data, int& offset) {
            TagInt arraySize = ReadInt(data, offset);
            TagByteArray result;
            result.reserve(arraySize);
            for (auto& el : result) {
                el = data.at(offset++);
            }
            return result;
        }

        static TagString ReadString(std::vector<unsigned char> data, int& offset) {
            short strLen = 0;
            strLen |= data.at(offset++) << 8;
            strLen |= data.at(offset++);
            std::string str(data.begin() + offset, data.begin()+offset+strLen);
            offset += strLen;
            return str;
        }

        static TagIntArray ReadIntArray(std::vector<unsigned char> data, int& offset) {
            TagInt arraySize = ReadInt(data, offset);
            TagIntArray result;
            for(int i = 0; i < arraySize; i++) {
                result.push_back(ReadInt(data, offset));
            }
            return result;
        }

        static TagLongArray ReadLongArray(std::vector<unsigned char> data, int& offset) {
            TagInt arraySize = ReadInt(data, offset);
            TagLongArray result;
            for(int i = 0; i < arraySize; i++) {
                result.push_back(ReadLong(data, offset));
            }
            return result;
        }

        static CompressionMode DetectCompression(std::vector<unsigned char> buf) {
            if (buf.size() < 2) {
                throw std::runtime_error("File is too small to determine compression");
            }

            if (buf.at(0) == 0x1f && buf.at(1) == 0x8B)
                return CompressionMode::ZLib;

            return CompressionMode::NONE;
        }
    };
}
#endif //D3PP_CPPNBT_H

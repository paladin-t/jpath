/*
** MIT License
**
** For the latest info, see https://github.com/paladin-t/jpath
**
** Copyright (C) 2020 Tony Wang
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
*/

#ifndef __JPATH_H__
#define __JPATH_H__

#include "rapidjson/include/rapidjson/document.h"
#include <string>

/*
** {========================================================
** Jpath
*/

class Jpath {
public:
	static bool read(const rapidjson::Value &obj, const rapidjson::Value* &ret, const char* node) {
		if (ret)
			ret = nullptr;
		if (!obj.IsObject() || !node)
			return false;
		auto entry = obj.FindMember(node);
		if (entry == obj.MemberEnd())
			return false;

		ret = &entry->value;

		return true;
	}
	static bool read(const rapidjson::Value &obj, const rapidjson::Value* &ret, int node) {
		if (ret)
			ret = nullptr;
		if (!obj.IsArray() || node < 0)
			return false;
		if ((rapidjson::SizeType)node >= obj.Capacity())
			return false;

		ret = &obj[node];

		return true;
	}
	template<typename Car, typename ...Cdr> static bool read(const rapidjson::Value &obj, const rapidjson::Value* &ret, Car car, Cdr ...cdr) {
		const rapidjson::Value* tmp = nullptr;
		if (!read(obj, tmp, car))
			return false;
		if (!tmp)
			return false;
		if (!read(*tmp, ret, cdr ...))
			return false;

		return true;
	}
	static bool write(rapidjson::Document &doc, rapidjson::Value &obj, rapidjson::Value* &ret, const char* node) {
		ret = nullptr;
		if (!node)
			return false;
		if (!obj.IsObject())
			obj.SetObject();
		auto entry = obj.FindMember(node);
		if (entry == obj.MemberEnd()) {
			rapidjson::Value key, val;
			key.SetString(node, doc.GetAllocator());
			val.SetNull();
			obj.AddMember(key, val, doc.GetAllocator());

			ret = &obj[node];
		} else {
			ret = &entry->value;
		}

		return true;
	}
	static bool write(rapidjson::Document &doc, rapidjson::Value &obj, rapidjson::Value* &ret, int node) {
		ret = nullptr;
		if (node < 0)
			return false;
		if (!obj.IsArray())
			obj.SetArray();
		while ((rapidjson::SizeType)node >= obj.Capacity()) {
			rapidjson::Value val;
			val.SetNull();
			obj.PushBack(val, doc.GetAllocator());
		}
		ret = &obj[node];

		return true;
	}
	template<typename Car, typename ...Cdr> static bool write(rapidjson::Document &doc, rapidjson::Value &obj, rapidjson::Value* &ret, Car car, Cdr ...cdr) {
		rapidjson::Value* tmp = nullptr;
		if (!write(doc, obj, tmp, car))
			return false;
		if (!write(doc, *tmp, ret, cdr ...))
			return false;

		return true;
	}

	template<typename Ret, typename ...List> static bool get(const rapidjson::Value &obj, Ret &ret, List ...list) {
		const rapidjson::Value* tmp = nullptr;
		if (!read(obj, tmp, list ...))
			return false;
		if (!tmp)
			return false;

		return getValue(*tmp, ret);
	}
	template<template <typename T, typename A = std::allocator<T> > typename Coll, typename Val, typename ...List> static bool get(const rapidjson::Value &obj, Coll<Val> &ret, List ...list) {
		const rapidjson::Value* tmp = nullptr;
		if (!read(obj, tmp, list ...))
			return false;
		if (!tmp)
			return false;
		if (!tmp->IsArray())
			return false;

		Coll<Val> result;
		for (rapidjson::SizeType i = 0; i < tmp->Capacity(); ++i) {
			Val val;
			if (!getValue(tmp[i], val))
				return false;

			result.push_back(val);
		}
		std::swap(result, ret);

		return true;
	}
	template<typename Src, typename ...List> static bool set(rapidjson::Document &doc, rapidjson::Value &obj, Src src, List ...list) {
		rapidjson::Value* tmp = nullptr;
		if (!write(doc, obj, tmp, list ...))
			return false;
		if (!tmp)
			return false;

		setValue(*tmp, src, doc);

		return true;
	}
	template<template <typename T, typename A = std::allocator<T> > typename Coll, typename Val, typename ...List> static bool set(rapidjson::Document &doc, rapidjson::Value &obj, const Coll<Val> &src, List ...list) {
		rapidjson::Value* tmp = nullptr;
		if (!write(doc, obj, tmp, list ...))
			return false;
		if (!tmp)
			return false;

		rapidjson::Value arr;
		arr.SetArray();
		for (auto it = src.begin(); it != src.end(); ++it)
			arr.PushBack(*it, doc.GetAllocator());
		setValue(*tmp, arr, doc);

		return true;
	}

private:
	static bool getValue(const rapidjson::Value &obj, bool &ret) {
		if (!obj.IsBool())
			return false;
		ret = obj.GetBool();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, int &ret) {
		if (!obj.IsNumber())
			return false;
		ret = obj.GetInt();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, unsigned &ret) {
		if (!obj.IsNumber())
			return false;
		ret = obj.GetUint();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, long long &ret) {
		if (!obj.IsNumber())
			return false;
		ret = obj.GetInt64();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, unsigned long long &ret) {
		if (!obj.IsNumber())
			return false;
		ret = obj.GetUint64();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, float &ret) {
		if (!obj.IsNumber())
			return false;
		ret = obj.GetFloat();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, double &ret) {
		if (!obj.IsNumber())
			return false;
		ret = obj.GetDouble();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, const char* &ret) {
		if (!obj.IsString())
			return false;
		ret = obj.GetString();

		return true;
	}
	static bool getValue(const rapidjson::Value &obj, std::string &ret) {
		if (!obj.IsString())
			return false;
		ret = obj.GetString();

		return true;
	}
	static void setValue(rapidjson::Value &obj, bool src, rapidjson::Document &) {
		obj.SetBool(src);
	}
	static void setValue(rapidjson::Value &obj, int src, rapidjson::Document &) {
		obj.SetInt(src);
	}
	static void setValue(rapidjson::Value &obj, unsigned src, rapidjson::Document &) {
		obj.SetUint(src);
	}
	static void setValue(rapidjson::Value &obj, long long src, rapidjson::Document &) {
		obj.SetInt64(src);
	}
	static void setValue(rapidjson::Value &obj, unsigned long long src, rapidjson::Document &) {
		obj.SetUint64(src);
	}
	static void setValue(rapidjson::Value &obj, float src, rapidjson::Document &) {
		obj.SetFloat(src);
	}
	static void setValue(rapidjson::Value &obj, double src, rapidjson::Document &) {
		obj.SetDouble(src);
	}
	static void setValue(rapidjson::Value &obj, const char* src, rapidjson::Document &doc) {
		obj.SetString(src, doc.GetAllocator());
	}
	static void setValue(rapidjson::Value &obj, const std::string &src, rapidjson::Document &doc) {
		obj.SetString(src.c_str(), doc.GetAllocator());
	}
	static void setValue(rapidjson::Value &obj, const rapidjson::Value &src, rapidjson::Document &doc) {
		obj.CopyFrom(src, doc.GetAllocator());
	}
};

/* ========================================================} */

#endif /* __JPATH_H__ */

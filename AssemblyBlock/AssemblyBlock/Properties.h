#pragma once

#define property_ro(GetFunc) __declspec(property(get = GetFunc))
#define property_rw(GetFunc, SetFunc) __declspec(property(get = GetFunc, put = SetFunc))

#ifndef __HEADER_UNITY_SHORTCUTS_H__
#define __HEADER_UNITY_SHORTCUTS_H__

#define BOILER()                                                                \
  void setUp(void) {}                                                          \
  void tearDown(void) {}

#define REGISTER(func) void func(void);

#endif

#pragma once

#include <CoreMinimal.h>


#define MakeSure(Condition) \
	if(!ensure(Condition)) return

#define MakeSureMsg(Condition, Format, ...) \
	if(!ensureMsgf(Condition, Format, ##__VA_ARGS__)) return

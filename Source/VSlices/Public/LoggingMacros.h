#pragma once

// Include the standard UE log header if not already
#include "CoreMinimal.h"

#define LOG_Warning(Format, ...) UE_LOG(LogTemp, Warning, TEXT(Format), ##__VA_ARGS__)
#define LOG_Error(Format, ...)   UE_LOG(LogTemp, Error, TEXT(Format), ##__VA_ARGS__)
#define LOG_Info(Format, ...)    UE_LOG(LogTemp, Display, TEXT(Format), ##__VA_ARGS__)

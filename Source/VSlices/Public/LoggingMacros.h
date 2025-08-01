#pragma once

// Include the standard UE log header if not already
#include "CoreMinimal.h"

#define LOG_WARNING(Format, ...) UE_LOG(LogTemp, Warning, TEXT(Format), ##__VA_ARGS__)
#define LOG_ERROR(Format, ...)   UE_LOG(LogTemp, Error, TEXT(Format), ##__VA_ARGS__)
#define LOG_INFO(Format, ...)    UE_LOG(LogTemp, Display, TEXT(Format), ##__VA_ARGS__)

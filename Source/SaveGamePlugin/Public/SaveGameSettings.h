// Copyright © 2025 created and maintained by wortback. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveGameSettings.generated.h"

/**
 * This file contains settings for the SaveGame Plugin.
 */
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig)
class SAVEGAMEPLUGIN_API USaveGameSettings : public UObject
{
	GENERATED_BODY()

public:
	/** Names of maps that needn't be saved in the game. Such maps will neither be saved nor loaded */
	UPROPERTY(EditAnywhere, Config, Category = "Map Transitions")
	TSet<FString> ExcludedFromSaveMaps;
};

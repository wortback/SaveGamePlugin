// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGamePlugin.h"
#if WITH_EDITOR
#include "Editor.h"     
#include "LevelEditor.h"      
#include "EngineUtils.h"       
#endif
#include "Interfaces/SerializableActor.h"
#include "LoggingCategories.h"

#define LOCTEXT_NAMESPACE "FSaveGamePluginModule"

void FSaveGamePluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
    MenuExtender->AddMenuExtension(
        "DataValidation",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FSaveGamePluginModule::AddMenuEntry)
    );

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
#endif
}

#if WITH_EDITOR
void FSaveGamePluginModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        FText::FromString("Validate Save System"),
        FText::FromString("Checks all actors in the current level for save system issues"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "DeveloperTools.MenuIcon"),
        FUIAction(FExecuteAction::CreateRaw(this, &FSaveGamePluginModule::OnValidateSaveSystem))
    );
}

void FSaveGamePluginModule::OnValidateSaveSystem()
{
    if (!GEditor) return;

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        UE_LOG(LogSaveSystem, Error, TEXT("No world found"));
        return;
    }

    int32 TotalActors = 0;
    int32 SerializableActors = 0;
    int32 ErrorCount = 0;
    TMap<FName, TArray<AActor*>> IDMap;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        TotalActors++;

        ISerializableActor* SerializableActor = Cast<ISerializableActor>(Actor);
        if (!SerializableActor) continue;
        SerializableActors++;

        FGuid Guid = SerializableActor->GetSerializableGUID();
        if (!Guid.IsValid())
        {
            GEditor->SelectActor(Actor, true, true);
            UE_LOG(LogSaveSystem, Error, TEXT("[SaveValidator] Actor %s has invalid GUID"), *Actor->GetName());
            ErrorCount++;
        }
        else
        {
            IDMap.FindOrAdd(SerializableActor->GetSerializableID()).Add(Actor);
        }

        if (SerializableActor->IsDynamicallySpawned())
        {
            UE_LOG(LogSaveSystem, Error, 
                TEXT("[SaveValidator] Actor %s has is marked dynamic but already exists in the map"), 
                *Actor->GetName());
            ErrorCount++;
        }
    }

    // Check for duplicates
    for (const auto& Pair : IDMap)
    {
        if (Pair.Value.Num() > 1)
        {
            UE_LOG(LogSaveSystem, Error, TEXT("[SaveValidator] Duplicate ID: %s"), *Pair.Key.ToString());
            for (AActor* Actor : Pair.Value)
            {
                UE_LOG(LogSaveSystem, Error, TEXT("  - %s"), *Actor->GetName());
            }
            ErrorCount++;
        }
    }

    UE_LOG(LogSaveSystem, Display, TEXT("========== Save System Validation =========="));
    UE_LOG(LogSaveSystem, Display, TEXT("Total Actors: %d"), TotalActors);
    UE_LOG(LogSaveSystem, Display, TEXT("Serializable Actors: %d"), SerializableActors);
    UE_LOG(LogSaveSystem, Display, TEXT("Errors Found: %d"), ErrorCount);
    UE_LOG(LogSaveSystem, Display, TEXT("=========================================="));
}

#endif

void FSaveGamePluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSaveGamePluginModule, SaveGamePlugin)
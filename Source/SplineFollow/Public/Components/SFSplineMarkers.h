#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "SFSplineMarkers.generated.h"

UENUM()
enum class ESWSplineMarkerType
{
    Single,
    Window
};

enum class ESWSplineMarkerProcessActionType
{
    Single,
    WindowStart,
    WindowEnd
};

struct FSWSplineMarkerProxy
{
    FSWSplineMarkerProxy( const float spline_length_percentage, const TFunction< void( AActor * ) > & function ) :
        SplineNormalizedDistance( spline_length_percentage ),
        Function( function )
    {}

    float SplineNormalizedDistance;
    TFunction< void( AActor * ) > Function;
};

UCLASS( Blueprintable )
class SPLINEFOLLOW_API USFSplineMarkerActorFilter : public UObject
{
    GENERATED_BODY()

public:
    UWorld * GetWorld() const override;

    UFUNCTION( BlueprintNativeEvent )
    bool CanSelectActor( AActor * actor );
};

UCLASS()
class SPLINEFOLLOW_API USFSplineMarkerActorFilter_SelectAll : public USFSplineMarkerActorFilter
{
    GENERATED_BODY()

public:
    bool CanSelectActor_Implementation( AActor * actor ) override;
};

UCLASS()
class SPLINEFOLLOW_API USFSplineMarkerActorFilter_SelectByClass : public USFSplineMarkerActorFilter
{
    GENERATED_BODY()

public:
    bool CanSelectActor_Implementation( AActor * actor ) override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< AActor > ClassToImplement;
};

USTRUCT( BlueprintType )
struct SPLINEFOLLOW_API FSWSplineMarkerInfos
{
    GENERATED_BODY()

    FSWSplineMarkerInfos() :
        Type( ESWSplineMarkerType::Single ),
        SingleActionNormalizedSplineDistance( 0.0f ),
        WindowStartNormalizedSplineDistance( 0.0f ),
        WindowEndNormalizedSplineDistance( 0.5f )
    {}

    UPROPERTY( EditAnywhere )
    ESWSplineMarkerType Type;

    UPROPERTY( EditAnywhere, meta = ( UIMin = 0.0f, UIMax = 1.0f, ClampMin = 0.0f, ClampMax = 1.0f, EditCondition = "Type == ESWSplineMarkerType::Single" ) )
    float SingleActionNormalizedSplineDistance;

    UPROPERTY( EditAnywhere, meta = ( UIMin = 0.0f, UIMax = 1.0f, ClampMin = 0.0f, ClampMax = 1.0f, EditCondition = "Type == ESWSplineMarkerType::Window" ) )
    float WindowStartNormalizedSplineDistance;

    UPROPERTY( EditAnywhere, meta = ( UIMin = 0.0f, UIMax = 1.0f, ClampMin = 0.0f, ClampMax = 1.0f, EditCondition = "Type == ESWSplineMarkerType::Window" ) )
    float WindowEndNormalizedSplineDistance;
};

UCLASS( Blueprintable, BlueprintType, meta = ( ShowWorldContextPin ) )
class SPLINEFOLLOW_API USFSplineMarkerAction : public UObject
{
    GENERATED_BODY()

public:
    UTexture2D * GetSprite() const;
    void ProcessAction( AActor * actor, ESWSplineMarkerProcessActionType action_type, const FSWSplineMarkerInfos & marker_infos ) const;
    UWorld * GetWorld() const override;

protected:
    UFUNCTION( BlueprintNativeEvent )
    void ExecuteAction( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteStartWindowAction( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteEndWindowAction( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

private:
    UPROPERTY( EditDefaultsOnly )
    UTexture2D * Sprite;

    UPROPERTY( EditAnywhere, meta = ( AllowAbstract = false ) )
    TArray< TSubclassOf< USFSplineMarkerActorFilter > > ActorFilters;
};

FORCEINLINE UTexture2D * USFSplineMarkerAction::GetSprite() const
{
    return Sprite;
}

UCLASS( Abstract )
class SPLINEFOLLOW_API ASWSplineMarkerLevelActor : public AActor
{
    GENERATED_BODY()

public:
    UTexture2D * GetSprite() const;
    void ProcessAction( AActor * actor, ESWSplineMarkerProcessActionType action_type, const FSWSplineMarkerInfos & marker_infos ) const;

protected:
    UFUNCTION( BlueprintNativeEvent )
    void ExecuteAction( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteStartWindowAction( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteEndWindowAction( AActor * actor, const FSWSplineMarkerInfos & marker_infos ) const;

private:
    UPROPERTY( EditDefaultsOnly )
    UTexture2D * Sprite;

    UPROPERTY( EditAnywhere, meta = ( AllowAbstract = false ) )
    TArray< TSubclassOf< USFSplineMarkerActorFilter > > ActorFilters;
};

FORCEINLINE UTexture2D * ASWSplineMarkerLevelActor::GetSprite() const
{
    return Sprite;
}

USTRUCT( BlueprintType )
struct SPLINEFOLLOW_API FSWSplineMarker
{
    GENERATED_BODY()

    FSWSplineMarker() :
        ItIsEnabled( true )
    {}

    virtual ~FSWSplineMarker() = default;

    virtual bool IsValid() const;
    virtual void AddSplineMarkerProxies( TArray< FSWSplineMarkerProxy > & proxies ) const;
    virtual UTexture2D * GetSprite() const;

    UPROPERTY( EditAnywhere )
    uint8 ItIsEnabled : 1;

    UPROPERTY( EditAnywhere, BlueprintReadonly )
    FSWSplineMarkerInfos Infos;
};

USTRUCT()
struct SPLINEFOLLOW_API FSWSplineMarker_Static : public FSWSplineMarker
{
    GENERATED_BODY()

    bool IsValid() const override;
    void AddSplineMarkerProxies( TArray< FSWSplineMarkerProxy > & proxies ) const override;
    UTexture2D * GetSprite() const override;

    UPROPERTY( EditAnywhere )
    TSubclassOf< USFSplineMarkerAction > ActionClass;
};

USTRUCT()
struct SPLINEFOLLOW_API FSWSplineMarker_LevelActor : public FSWSplineMarker
{
    GENERATED_BODY()

    FSWSplineMarker_LevelActor();

    bool IsValid() const override;
    void AddSplineMarkerProxies( TArray< FSWSplineMarkerProxy > & proxies ) const override;
    UTexture2D * GetSprite() const override;

    UPROPERTY( EditAnywhere )
    ASWSplineMarkerLevelActor * LevelActor;
};

USTRUCT( Blueprintable, BlueprintType )
struct SPLINEFOLLOW_API FSWSplineMarker_Data : public FSWSplineMarker
{
    GENERATED_BODY()

    UTexture2D * GetSprite() const override;

    UPROPERTY( EditAnywhere )
    UTexture2D * Sprite;

    UPROPERTY( EditAnywhere, BlueprintReadonly )
    TObjectPtr< UObject > Data;
};

FORCEINLINE UTexture2D * FSWSplineMarker_Data::GetSprite() const
{
    return Sprite;
}
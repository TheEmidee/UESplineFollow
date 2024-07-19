#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "SFSplineMarkers.generated.h"

UENUM( BlueprintType )
enum class ESFSplineMarkerType : uint8
{
    Single,
    Window
};

enum class ESFSplineMarkerProcessActionType
{
    Single,
    WindowStart,
    WindowEnd
};

struct FSFSplineMarkerProxy
{
    FSFSplineMarkerProxy( const float spline_length_percentage, const TFunction< void( AActor * ) > & function ) :
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
struct SPLINEFOLLOW_API FSFSplineMarkerInfos
{
    GENERATED_BODY()

    FSFSplineMarkerInfos() :
        Type( ESFSplineMarkerType::Single ),
        SingleActionNormalizedSplineDistance( 0.0f ),
        WindowStartNormalizedSplineDistance( 0.0f ),
        WindowEndNormalizedSplineDistance( 0.5f )
    {}

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    ESFSplineMarkerType Type;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( UIMin = 0.0f, UIMax = 1.0f, ClampMin = 0.0f, ClampMax = 1.0f, EditCondition = "Type == ESFSplineMarkerType::Single" ) )
    float SingleActionNormalizedSplineDistance;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( UIMin = 0.0f, UIMax = 1.0f, ClampMin = 0.0f, ClampMax = 1.0f, EditCondition = "Type == ESFSplineMarkerType::Window" ) )
    float WindowStartNormalizedSplineDistance;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( UIMin = 0.0f, UIMax = 1.0f, ClampMin = 0.0f, ClampMax = 1.0f, EditCondition = "Type == ESFSplineMarkerType::Window" ) )
    float WindowEndNormalizedSplineDistance;
};

UCLASS( Blueprintable, BlueprintType, meta = ( ShowWorldContextPin ) )
class SPLINEFOLLOW_API USFSplineMarkerAction : public UObject
{
    GENERATED_BODY()

public:
    UTexture2D * GetSprite() const;
    void ProcessAction( AActor * actor, ESFSplineMarkerProcessActionType action_type, const FSFSplineMarkerInfos & marker_infos ) const;
    UWorld * GetWorld() const override;

protected:
    UFUNCTION( BlueprintNativeEvent )
    void ExecuteAction( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteStartWindowAction( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteEndWindowAction( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const;

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
class SPLINEFOLLOW_API ASFSplineMarkerLevelActor : public AActor
{
    GENERATED_BODY()

public:
    UTexture2D * GetSprite() const;
    void ProcessAction( AActor * actor, ESFSplineMarkerProcessActionType action_type, const FSFSplineMarkerInfos & marker_infos ) const;

protected:
    UFUNCTION( BlueprintNativeEvent )
    void ExecuteAction( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteStartWindowAction( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const;

    UFUNCTION( BlueprintNativeEvent )
    void ExecuteEndWindowAction( AActor * actor, const FSFSplineMarkerInfos & marker_infos ) const;

private:
    UPROPERTY( EditDefaultsOnly )
    UTexture2D * Sprite;

    UPROPERTY( EditAnywhere, meta = ( AllowAbstract = false ) )
    TArray< TSubclassOf< USFSplineMarkerActorFilter > > ActorFilters;
};

FORCEINLINE UTexture2D * ASFSplineMarkerLevelActor::GetSprite() const
{
    return Sprite;
}

UCLASS( DefaultToInstanced, Blueprintable, BlueprintType, Abstract, EditInlineNew )
class SPLINEFOLLOW_API USFSplineMarkerObject : public UObject
{
    GENERATED_BODY()

public:
    virtual void AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies, const FSFSplineMarkerInfos & infos ) const;

    UPROPERTY( BlueprintReadOnly, EditAnywhere )
    TObjectPtr< UTexture2D > Sprite;

    UPROPERTY( BlueprintReadOnly, EditAnywhere )
    FLinearColor Color = FLinearColor::White;
};

UCLASS()
class SPLINEFOLLOW_API USFSplineMarkerObject_Action : public USFSplineMarkerObject
{
    GENERATED_BODY()

public:
    void AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies, const FSFSplineMarkerInfos & infos ) const override;

    UPROPERTY( BlueprintReadOnly, EditAnywhere )
    TSubclassOf< USFSplineMarkerAction > ActionClass;
};

UCLASS()
class SPLINEFOLLOW_API USFSplineMarkerObject_Data : public USFSplineMarkerObject
{
    GENERATED_BODY()

public:
    UPROPERTY( BlueprintReadOnly, EditAnywhere )
    TSubclassOf< USFSplineMarkerData > Data;
};

UCLASS( Abstract, Blueprintable, BlueprintType )
class SPLINEFOLLOW_API USFSplineMarkerData : public UObject
{
    GENERATED_BODY()
};

UCLASS()
class SPLINEFOLLOW_API USFSplineMarkerObject_LevelActor : public USFSplineMarkerObject
{
    GENERATED_BODY()

public:
    void AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies, const FSFSplineMarkerInfos & infos ) const override;

    UPROPERTY( BlueprintReadOnly, EditAnywhere )
    TObjectPtr< ASFSplineMarkerLevelActor > LevelActor;
};

USTRUCT( BlueprintType, Blueprintable )
struct SPLINEFOLLOW_API FSFSplineMarker
{
    GENERATED_BODY()

    FSFSplineMarker() :
        ItIsEnabled( true )
    {
        Name = TEXT( "Marker" );
    }

    FSFSplineMarker( const FName & name, uint8 it_is_enable, const FSFSplineMarkerInfos & infos, const TObjectPtr< USFSplineMarkerObject > & object ) :
        Name( name ),
        ItIsEnabled( it_is_enable ),
        Infos( infos ),
        Object( object )
    {}

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    FName Name;

    virtual ~FSFSplineMarker() = default;

    virtual bool IsValid() const;
    virtual void AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies ) const;
    virtual UTexture2D * GetSprite() const;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    uint8 ItIsEnabled : 1;

    UPROPERTY( EditAnywhere, BlueprintReadonly )
    FSFSplineMarkerInfos Infos;

    UPROPERTY( EditAnywhere, Instanced, BlueprintReadonly )
    TObjectPtr< USFSplineMarkerObject > Object;

    bool operator==( const auto & other ) const
    {
        return Name == other.Name && Infos.SingleActionNormalizedSplineDistance == other.Infos.SingleActionNormalizedSplineDistance;
    }
};

FORCEINLINE uint32 GetTypeHash( const FSFSplineMarker & spline_marker )
{
    return HashCombine( GetTypeHash( spline_marker.Name ), GetTypeHash( spline_marker.Infos.SingleActionNormalizedSplineDistance ) );
}

USTRUCT()
struct SPLINEFOLLOW_API FSFSplineMarker_Static : public FSFSplineMarker
{
    GENERATED_BODY()

    bool IsValid() const override;
    void AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies ) const override;
    UTexture2D * GetSprite() const override;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSubclassOf< USFSplineMarkerAction > ActionClass;
};

USTRUCT()
struct SPLINEFOLLOW_API FSFSplineMarker_LevelActor : public FSFSplineMarker
{
    GENERATED_BODY()

    FSFSplineMarker_LevelActor();

    bool IsValid() const override;
    void AddSplineMarkerProxies( TArray< FSFSplineMarkerProxy > & proxies ) const override;
    UTexture2D * GetSprite() const override;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    ASFSplineMarkerLevelActor * LevelActor;
};

USTRUCT( Blueprintable, BlueprintType )
struct SPLINEFOLLOW_API FSFSplineMarker_Data : public FSFSplineMarker
{
    GENERATED_BODY()

    FSFSplineMarker_Data();

    UTexture2D * GetSprite() const override;

    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    UTexture2D * Sprite;

    UPROPERTY( EditAnywhere, BlueprintReadonly )
    TSubclassOf< UObject > Data;
};

FORCEINLINE UTexture2D * FSFSplineMarker_Data::GetSprite() const
{
    return Sprite;
}
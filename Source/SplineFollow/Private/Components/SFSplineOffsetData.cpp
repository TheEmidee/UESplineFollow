#include "Components/SFSplineOffsetData.h"

USFSplineOffsetData::USFSplineOffsetData()
{
    bResetOnEnd = true;
    OffsetType = ESFSplineOffsetType::Location;
    ElapsedTime = 0.0f;
    MaxTime = 0.0f;
}

void USFSplineOffsetData::Initialize()
{
    auto time = 0.0f;

    for ( auto curve_index = 0; curve_index < 3; ++curve_index )
    {
        auto min_time = 0.0f;
        auto max_time = 0.0f;

        const auto curve = OffsetCurve.GetRichCurve( curve_index );
        curve->GetTimeRange( min_time, max_time );

        if ( max_time > time )
        {
            time = max_time;
        }
    }

    MaxTime = time;
}

void USFSplineOffsetData::GetOffsetTransform( FTransform & transform, const float delta_time )
{
    transform.SetIdentity();

    if ( ElapsedTime >= MaxTime )
    {
        OnSplineOffsetFinishedDelegate.Broadcast( this );
        return;
    }

    ElapsedTime += delta_time;

    const auto offset = OffsetCurve.GetValue( ElapsedTime );

    switch ( OffsetType )
    {
        case ESFSplineOffsetType::Location:
        {
            transform.SetLocation( offset );
        }
        break;
        case ESFSplineOffsetType::Rotation:
        {
            transform.SetRotation( FRotator( offset.X, offset.Y, offset.Z ).Quaternion() );
        }
        break;
        case ESFSplineOffsetType::Scale:
        {
            transform.SetScale3D( offset );
        }
        break;
        default:
        {
            checkNoEntry();
        }
    }
}
#include "Components/SFSplineOffsetData.h"

USFSplineOffsetData::USFSplineOffsetData()
{
    bResetOnEnd = true;
    OffsetType = ESFSplineOffsetType::Location;
    ElapsedTime = 0.0f;
}

void USFSplineOffsetData::GetOffsetTransform( FTransform & transform, const float delta_time )
{
    transform.SetIdentity();

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
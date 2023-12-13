#include "SplineFollowModule.h"

class FSplineFollowModule final : public ISplineFollowModule
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

IMPLEMENT_MODULE( FSplineFollowModule, SplineFollow )

void FSplineFollowModule::StartupModule()
{
}

void FSplineFollowModule::ShutdownModule()
{
}
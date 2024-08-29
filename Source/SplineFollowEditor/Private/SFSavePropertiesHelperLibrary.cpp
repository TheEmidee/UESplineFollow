#include "SFSavePropertiesHelperLibrary.h"

void USFSavePropertiesHelperLibrary::GetFloat( const FString & section, const FString & key, UPARAM( ref ) float & value )
{
    GConfig->GetFloat( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::SaveFloat( const FString & section, const FString & key, UPARAM( ref ) float & value )
{
    GConfig->SetFloat( *section, *key, value, GEditorPerProjectIni );
}

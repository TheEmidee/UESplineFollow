#include "SFSavePropertiesHelperLibrary.h"

void USFSavePropertiesHelperLibrary::GetArray( const FString & section, const FString & key, TArray< FString > & value )
{
    GConfig->GetArray( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::SaveArray( const FString & section, const FString & key, TArray< FString > & value )
{
    GConfig->SetArray( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::GetBool( const FString & section, const FString & key, bool & value )
{
    GConfig->GetBool( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::SaveBool( const FString & section, const FString & key, bool & value )
{
    GConfig->SetBool( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::GetColor( const FString & section, const FString & key, FColor & value )
{
    GConfig->GetColor( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::SaveColor( const FString & section, const FString & key, FColor & value )
{
    GConfig->SetColor( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::GetFloat( const FString & section, const FString & key, UPARAM( ref ) float & value )
{
    GConfig->GetFloat( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::SaveFloat( const FString & section, const FString & key, UPARAM( ref ) float & value )
{
    GConfig->SetFloat( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::GetString( const FString & section, const FString & key, FString & value )
{
    GConfig->GetString( *section, *key, value, GEditorPerProjectIni );
}

void USFSavePropertiesHelperLibrary::SaveString( const FString & section, const FString & key, FString & value )
{
    GConfig->SetString( *section, *key, *value, GEditorPerProjectIni );
}

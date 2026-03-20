


#include "CryptoDonations.h"

#define LOCTEXT_NAMESPACE "FCryptoDonationsModule"

void FCryptoDonationsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FCryptoDonationsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
//#if WITH_EDITOR
//IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, CryptoDonations, "CryptoDonations")
//#else
IMPLEMENT_MODULE(FCryptoDonationsModule, CryptoDonations)
//#endif
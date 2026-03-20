#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CryptoDonationsSubsystem.h"
#include "CryptoDonationWidget.generated.h"

UCLASS()
class CRYPTODONATIONS_API UCryptoDonationWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    virtual void NativeConstruct() override;

    // === Blueprint callable ===
    UFUNCTION(BlueprintCallable)
    void StartDonation(float Amount, FString Currency, FString UserId);

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void OnQRCodeReady(UTexture2D* Texture);

    UFUNCTION(BlueprintImplementableEvent)
    void OnPaymentCreatedBP(const FDonationSession& Session);

    UFUNCTION(BlueprintImplementableEvent)
    void OnPaymentConfirmedBP(int32 Reward);

    UFUNCTION(BlueprintImplementableEvent)
    void OnPaymentFailedBP(const FString& Error);

private:

    UPROPERTY()
    UCryptoDonationsSubsystem* Subsystem;

    // Internal handlers
    UFUNCTION()
    void HandlePaymentCreated(const FDonationSession& Session);
    UFUNCTION()
    void HandlePaymentConfirmed(int32 Reward);
    UFUNCTION()
    void HandlePaymentFailed(const FString& Error);

	void DownloadQRCode(FString URL);
	void OnQRDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};

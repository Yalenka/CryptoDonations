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
    void OnPaymentCreatedBP(const FDonationSession& Session);

    UFUNCTION(BlueprintImplementableEvent)
    void OnPaymentConfirmedBP(int32 Reward);

    UFUNCTION(BlueprintImplementableEvent)
    void OnPaymentFailedBP(const FString& Error);

private:

    UPROPERTY()
    UCryptoDonationsSubsystem* Subsystem;

    // Internal handlers
    void HandlePaymentCreated(const FDonationSession& Session);
    void HandlePaymentConfirmed(int32 Reward);
    void HandlePaymentFailed(const FString& Error);
};
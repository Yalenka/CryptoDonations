#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CryptoAPIClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentCreated, const FDonationSession&, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaymentStatus, const FString&, Status);

UCLASS()
class CRYPTODONATIONS_API UCryptoAPIClient : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FOnPaymentCreated OnPaymentCreated;

    UPROPERTY(BlueprintAssignable)
    FOnPaymentStatus OnPaymentStatus;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crypto")
    FString BackendURL = "http://127.0.0.1:3000";

    UFUNCTION(BlueprintCallable)
    void CreatePayment(float Amount, FString Currency, FString UserId);

    UFUNCTION(BlueprintCallable)
    void CheckPaymentStatus(FString PaymentID);
    void OnCreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
    void OnStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
};

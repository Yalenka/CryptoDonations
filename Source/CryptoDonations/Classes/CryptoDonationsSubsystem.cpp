#include "CryptoDonationsSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

void UCryptoDonationsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

// ===============================
// START DONATION
// ===============================
void UCryptoDonationsSubsystem::StartDonation(float Amount, FString Currency, FString UserId)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + TEXT("/create-payment"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    FString Body = FString::Printf(
        TEXT("{\"amount\": %.2f, \"currency\": \"%s\", \"userId\": \"%s\"}"),
        Amount,
        *Currency,
        *UserId
    );

    Request->SetContentAsString(Body);

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoDonationsSubsystem::CreatePaymentResponse);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("📡 Sending payment request..."));
}

// ===============================
// CREATE RESPONSE
// ===============================
void UCryptoDonationsSubsystem::CreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CreatePayment failed"));
        OnPaymentFailed.Broadcast(TEXT("Network error"));
        return;
    }

    FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("CreatePayment Response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OnPaymentFailed.Broadcast(TEXT("Invalid server response"));
        return;
    }

    FDonationSession Session;

    if (JsonObject->HasField(TEXT("payment_id")))
        Session.PaymentID = JsonObject->GetStringField(TEXT("payment_id"));

    if (JsonObject->HasField(TEXT("pay_address")))
        Session.PayAddress = JsonObject->GetStringField(TEXT("pay_address"));

    if (JsonObject->HasField(TEXT("pay_amount")))
        Session.PayAmount = JsonObject->GetStringField(TEXT("pay_amount"));

    if (JsonObject->HasField(TEXT("qr_code_url")))
        Session.QRCodeURL = JsonObject->GetStringField(TEXT("qr_code_url"));

    CurrentPaymentID = Session.PaymentID;

    OnPaymentCreated.Broadcast(Session);

    // Start polling every 5 seconds
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            PollTimer,
            this,
            &UCryptoDonationsSubsystem::CheckPaymentStatus,
            5.0f,
            true
        );
    }
}

// ===============================
// POLLING
// ===============================
void UCryptoDonationsSubsystem::CheckPaymentStatus()
{
    if (CurrentPaymentID.IsEmpty()) return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + TEXT("/payment-status/") + CurrentPaymentID);
    Request->SetVerb(TEXT("GET"));

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoDonationsSubsystem::PaymentStatusResponse);
    Request->ProcessRequest();
}

// ===============================
// STATUS RESPONSE
// ===============================
void UCryptoDonationsSubsystem::PaymentStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid()) return;

    FString ResponseStr = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("Status Response: %s"), *ResponseStr);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) return;

    FString Status = TEXT("unknown");
    int32 Reward = 0;

    if (JsonObject->HasField(TEXT("status")))
        Status = JsonObject->GetStringField(TEXT("status"));

    if (JsonObject->HasField(TEXT("reward")))
        Reward = JsonObject->GetIntegerField(TEXT("reward"));

    if (Status == TEXT("finished"))
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(PollTimer);
        }

        UE_LOG(LogTemp, Log, TEXT("Payment finished, reward: %d"), Reward);

        OnPaymentConfirmed.Broadcast(Reward);
    }
    else if (Status == TEXT("failed"))
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(PollTimer);
        }

        UE_LOG(LogTemp, Error, TEXT("Payment failed"));

        OnPaymentFailed.Broadcast(TEXT("Payment failed"));
    }
}

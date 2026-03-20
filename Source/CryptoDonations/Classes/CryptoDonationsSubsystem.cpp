#include "CryptoDonationsSubsystem.h"
void UCryptoDonationsSubsystem::CreatePaymentResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid())
    {
        OnPaymentFailed.Broadcast("Network error");
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        OnPaymentFailed.Broadcast("Invalid server response");
        return;
    }

    FDonationSession Session;
    Session.PaymentID = JsonObject->GetStringField("payment_id");
    Session.PayAddress = JsonObject->GetStringField("pay_address");
    Session.PayAmount = JsonObject->GetStringField("pay_amount");
    Session.QRCodeURL = JsonObject->GetStringField("qr_code_url");

    CurrentPaymentID = Session.PaymentID;

    OnPaymentCreated.Broadcast(Session);

    GetWorld()->GetTimerManager().SetTimer(
        PollTimer,
        this,
        &UCryptoDonationsSubsystem::CheckPaymentStatus,
        5.0f,
        true
    );
}

void UCryptoDonationsSubsystem::CheckPaymentStatus()
{
    if (CurrentPaymentID.IsEmpty()) return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(BackendURL + "/payment-status/" + CurrentPaymentID);
    Request->SetVerb("GET");

    Request->OnProcessRequestComplete().BindUObject(this, &UCryptoDonationsSubsystem::PaymentStatusResponse);
    Request->ProcessRequest();
}

void UCryptoDonationsSubsystem::PaymentStatusResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (!bSuccess || !Response.IsValid()) return;

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObject)) return;

    FString Status = JsonObject->GetStringField("status");

    if (Status == "finished")
    {
        GetWorld()->GetTimerManager().ClearTimer(PollTimer);

        int32 Reward = JsonObject->GetIntegerField("reward");

        OnPaymentConfirmed.Broadcast(Reward);
    }
    else if (Status == "failed")
    {
        GetWorld()->GetTimerManager().ClearTimer(PollTimer);
        OnPaymentFailed.Broadcast("Payment failed");
    }
}
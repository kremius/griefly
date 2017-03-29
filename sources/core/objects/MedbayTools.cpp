#include "MedbayTools.h"

HealthAnalyzer::HealthAnalyzer(quint32 id) : Item(id)
{
    SetSprite("icons/device.dmi");
    SetState("health");
    name = "Health Analyzer";
}

void HealthAnalyzer::Scan(IdPtr<Human> target)
{
    float health = target->GetHealth() / 100.0f;
    float suffocation = target->GetSuffocationDamage() / 100.0f;
    float burn = target->GetBurnDamage() / 100.0f;
    float brute = target->GetBruteDamage() / 100.0f;
	IChat& GameChat = GetGame().GetChat();
    GameChat.PostHtmlFor(
        QString("<b>Analyzing results for %1:</b>").arg(target->name.toHtmlEscaped()),
        GetOwner());
    GameChat.PostTextFor("Overall status:", GetOwner());

    QString healthy = QString("<b>%1%</b> healthy").arg(health);
    QString deceased("<b><font color=\"red\">Deceased</font></b>");

    GameChat.PostHtmlFor(target->IsDead() ? deceased : healthy, GetOwner());
    if (target->GetBruteDamage() > HUMAN_MAX_HEALTH / 10)
    {
        QString level = target->GetBruteDamage() > (HUMAN_MAX_HEALTH / 2) ? "Severe" : "Minor";
        GameChat.PostHtmlFor(
            QString("%1 <font color=\"red\">tissue damage</font> detected.").arg(level),
            GetOwner());
    }
    if (target->GetBurnDamage() > HUMAN_MAX_HEALTH / 10)
    {
        QString level = target->GetBurnDamage() > (HUMAN_MAX_HEALTH / 2) ? "Severe" : "Minor";
        GameChat.PostHtmlFor(
            QString("%1 <font color=\"#7f8200\">burn damage</font> detected.").arg(level),
            GetOwner());
    }
    if (target->GetSuffocationDamage() > HUMAN_MAX_HEALTH / 10)
    {
        QString level = target->GetSuffocationDamage() > HUMAN_MAX_HEALTH / 2 ? "Severe" : "Minor";
        GameChat.PostHtmlFor(
            QString("%1 <font color=\"blue\">oxygen deprivation</font> detected.").arg(level),
            GetOwner());
    }
    GameChat.PostHtmlFor(
        QString("Damage: <font color=\"red\">Brute</font>-"
                "<font color=\"#7f8200\">Burn</font>-"
                "<font color=\"blue\">Suffocation</font>"
                " Specfics: <b><font color=\"red\">%1%</font></b>-"
                "<b><font color=\"#7f8200\">%2%</font></b>-"
                "<b><font color=\"blue\">%3%</font></b><br>")
            .arg(brute).arg(burn).arg(suffocation),
        GetOwner());
}
Medicine::Medicine(quint32 id) : Item(id)
{
    burn_heal_ = 0;
    brute_heal_ = 0;
}

void Medicine::Heal(IdPtr<Human> target)
{
    target->ApplyBurnDamage(-1 * burn_heal_);
    target->ApplyBruteDamage(-1 * brute_heal_);
    Delete();
}

Ointment::Ointment(quint32 id) : Medicine(id)
{
    SetState("ointment");
    burn_heal_ = 1000;
    name = "Ointment";
}

BruisePack::BruisePack(quint32 id) : Medicine(id)
{
    SetState("brutepack");
    brute_heal_ = 1000;
    name = "bruise pack";
}

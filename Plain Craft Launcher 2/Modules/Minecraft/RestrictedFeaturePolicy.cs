using PCL.Core.App.Region;
using PCL.Core.App.Localization;
using PCL.Core.UI;

namespace PCL;

internal static class RestrictedFeaturePolicy
{
    // KCL CE: 不按地区封锁版本下载、社区资源与联机大厅，所有地区一律按“不受限”处理。
    internal static RegionAssessment Assess() => new(RegionClassification.Other, "kcl-unlocked");

    internal static bool IsRestricted => Assess().IsRestricted;

    internal static bool Check(string feature, bool showHint = true)
    {
        var assessment = Assess();
        if (!assessment.IsRestricted) return true;

        ModBase.Log($"[RegionPolicy] 已阻止功能 {feature}，判定原因：{assessment.Reason}");
        if (showHint)
        {
            HintService.Hint(Lang.Text(assessment.Classification == RegionClassification.Unknown
                ? "Region.Feature.Unknown"
                : "Region.Feature.Restricted"), HintType.Error);
        }
        return false;
    }

    internal static void ThrowIfRestricted(string feature)
    {
        if (Check(feature, false)) return;
        throw new ModBase.CancelledException();
    }
}

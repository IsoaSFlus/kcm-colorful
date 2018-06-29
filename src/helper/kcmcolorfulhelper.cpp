#include "kcmcolorfulhelper.h"
#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <QtGlobal>
#include <QDir>

KcmColorfulHelper::KcmColorfulHelper(int argc, char *argv[], QObject *parent) : QObject(parent)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    mConfig = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    wallpaperFilePath = QString(argv[1]);
//    genCSName();
//    getPrevCSName();
//    readDefaultCS();
}

KcmColorfulHelper::~KcmColorfulHelper()
{
    delete c;
    colorExtractProc->terminate();
    colorExtractProc->waitForFinished(3000);
    colorExtractProc->deleteLater();
    mConfig->markAsClean();
    tConfig->markAsClean();
}

void KcmColorfulHelper::run()
{
    QStringList arg;

    colorExtractProc = new QProcess(this);

    arg.append("-c");
    arg.append("exec(\"\"\"\\nfrom colorthief import ColorThief\\nimport sys\\ncolor_thief = ColorThief(sys.argv[1])\\ndominant_color = color_thief.get_color(quality=5)\\nprint(\"IsoaSFlus,{},{},{}\".format(dominant_color[0], dominant_color[1], dominant_color[2]))\\nsys.stdout.flush()\\n\"\"\")");
    arg.append(wallpaperFilePath);

    connect(colorExtractProc, &QProcess::readyReadStandardOutput, this, &KcmColorfulHelper::dealStdOut);
    colorExtractProc->start("python3", arg);
}

void KcmColorfulHelper::getPrevCSName()
{
    KSharedConfigPtr cfg = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    KConfigGroup groupOut(cfg, "General");
    prevColorSchemeName = groupOut.readEntry("ColorScheme");

}

void KcmColorfulHelper::readTemplateCS()
{
    QString tName;
    if (isDarkTheme()) {
        tName = "ColorfulDark";
    } else {
        tName = "ColorfulLight";
    }

    QString schemeFile;

    schemeFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/" + tName + ".colors");
    QFile file(schemeFile);

    if (!file.exists()) {
        schemeFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                    + "/color-schemes/" + "BreezeIso.colors";
    }
    qDebug() << schemeFile;
    tConfig = KSharedConfig::openConfig(schemeFile);
}

void KcmColorfulHelper::changeColorScheme()
{
    KConfigGroup group(mConfig, "Colors:View");
//    qDebug() << group.readEntry("ForegroundNormal");

    group.writeEntry("ForegroundNormal", addJitter(group.readEntry("ForegroundNormal")));
    group.writeEntry("DecorationHover", *c);


//    KConfigGroup groupWMTheme(config, "WM");
    KConfigGroup groupWMOut(mConfig, "WM");

    QStringList colorItemListWM;
    colorItemListWM << "activeBackground"
                    << "activeForeground"
                    << "inactiveBackground"
                    << "inactiveForeground";
//                    << "activeBlend"
//                    << "inactiveBlend";

    QVector<QColor> defaultWMColors;
    defaultWMColors << *c
                    << QColor(239,240,241)
                    << *c
                    << QColor(189,195,199)
/*                    << QColor(255,255,255)
                    << QColor(75,71,67)*/;

    int i = 0;
    for (const QString &coloritem : colorItemListWM)
    {
            groupWMOut.writeEntry(coloritem, defaultWMColors.value(i));
            ++i;
    }
}

void KcmColorfulHelper::changeColorScheme(KSharedConfigPtr config)
{
//    // store colorscheme name in global settings
//    mConfig = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
//    KConfigGroup groupOut(mConfig, "General");
//    qDebug() << groupOut.readEntry("ColorScheme");
//    groupOut.writeEntry("ColorScheme", colorSchemeName);

    QStringList colorItemList;
    colorItemList << "BackgroundNormal"
                  << "BackgroundAlternate"
                  << "ForegroundNormal"
                  << "ForegroundInactive"
                  << "ForegroundActive"
                  << "ForegroundLink"
                  << "ForegroundVisited"
                  << "ForegroundNegative"
                  << "ForegroundNeutral"
                  << "ForegroundPositive"
                  << "DecorationFocus"
                  << "DecorationHover";

    QStringList colorSetGroupList;
    colorSetGroupList << "Colors:View"
                      << "Colors:Window"
                      << "Colors:Button"
                      << "Colors:Selection"
                      << "Colors:Tooltip"
                      << "Colors:Complementary";

    QList <KColorScheme> colorSchemes;

    colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::View, config));
    colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Window, config));
    colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Button, config));
    colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Selection, config));
    colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Tooltip, config));
    colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Complementary, config));

    for (int i = 0; i < colorSchemes.length(); ++i)
    {
        KConfigGroup group(mConfig, colorSetGroupList.value(i));
        group.writeEntry("BackgroundNormal", colorSchemes[i].background(KColorScheme::NormalBackground).color());
        group.writeEntry("BackgroundAlternate", colorSchemes[i].background(KColorScheme::AlternateBackground).color());
        group.writeEntry("ForegroundNormal",addJitter(colorSchemes[i].foreground(KColorScheme::NormalText).color()));
        group.writeEntry("ForegroundInactive", colorSchemes[i].foreground(KColorScheme::InactiveText).color());
        group.writeEntry("ForegroundActive", colorSchemes[i].foreground(KColorScheme::ActiveText).color());
        group.writeEntry("ForegroundLink", colorSchemes[i].foreground(KColorScheme::LinkText).color());
        group.writeEntry("ForegroundVisited", colorSchemes[i].foreground(KColorScheme::VisitedText).color());
        group.writeEntry("ForegroundNegative", colorSchemes[i].foreground(KColorScheme::NegativeText).color());
        group.writeEntry("ForegroundNeutral", colorSchemes[i].foreground(KColorScheme::NeutralText).color());
        group.writeEntry("ForegroundPositive", colorSchemes[i].foreground(KColorScheme::PositiveText).color());
        group.writeEntry("DecorationFocus", colorSchemes[i].decoration(KColorScheme::FocusColor).color());
        if (i == 0) {
           group.writeEntry("DecorationHover", *c);
        } else {
            group.writeEntry("DecorationHover", colorSchemes[i].decoration(KColorScheme::HoverColor).color());
        }
    }

    KConfigGroup groupWMTheme(config, "WM");
    KConfigGroup groupWMOut(mConfig, "WM");

    QStringList colorItemListWM;
    colorItemListWM << "activeBackground"
                    << "activeForeground"
                    << "inactiveBackground"
                    << "inactiveForeground"
                    << "activeBlend"
                    << "inactiveBlend";

    QVector<QColor> defaultWMColors;
    defaultWMColors << *c
                    << colorSchemes[1].foreground(KColorScheme::NormalText).color()
                    << *c
                    << colorSchemes[1].foreground(KColorScheme::NormalText).color()
                    << QColor(255,255,255)
                    << QColor(75,71,67);

    int i = 0;
    for (const QString &coloritem : colorItemListWM)
    {
        groupWMOut.writeEntry(coloritem, defaultWMColors.value(i));
        ++i;
    }

    QStringList groupNameList;
    groupNameList << "ColorEffects:Inactive" << "ColorEffects:Disabled";

    QStringList effectList;
    effectList << "Enable"
               << "ChangeSelectionColor"
               << "IntensityEffect"
               << "IntensityAmount"
               << "ColorEffect"
               << "ColorAmount"
               << "Color"
               << "ContrastEffect"
               << "ContrastAmount";

    for (const QString &groupName : groupNameList)
    {

        KConfigGroup groupEffectOut(mConfig, groupName);
        KConfigGroup groupEffectTheme(config, groupName);

        for (const QString &effect : effectList) {
            groupEffectOut.writeEntry(effect, groupEffectTheme.readEntry(effect));
        }
    }
}

void KcmColorfulHelper::save()
{
    mConfig->sync();

//    KConfig cfg(QStringLiteral("kcmdisplayrc"), KConfig::NoGlobals);
//    KConfigGroup displayGroup(&cfg, "X11");

//    displayGroup.writeEntry("exportKDEColors", true);

//    cfg.sync();

//    QSettings* settings = new QSettings(QLatin1String("Trolltech"));
//    KSharedConfigPtr kglobalcfg = KSharedConfig::openConfig( "kdeglobals" );
//    KConfigGroup kglobals(kglobalcfg, "KDE");
//    QPalette newPal = KColorScheme::createApplicationPalette(kglobalcfg);
//    applyQtColors(kglobalcfg, *settings, newPal);
//    delete settings;
//    runRdb(KRdbExportQtColors | KRdbExportGtkTheme |  KRdbExportColors);

    QDBusMessage messageA = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"), QStringLiteral("org.kde.KGlobalSettings"), QStringLiteral("notifyChange") );
    QList<QVariant> args;
    args.append(0);//previous KGlobalSettings::PaletteChanged. This is now private API in khintsettings
    args.append(0);//unused in palette changed but needed for the DBus signature
    messageA.setArguments(args);
    QDBusConnection::sessionBus().send(messageA);
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"));
    QDBusConnection::sessionBus().send(message);
    //    emit changed(false);
}

void KcmColorfulHelper::genCSName()
{
    colorSchemeName = "" + QUuid::createUuid().toString(QUuid::WithoutBraces).mid(0, 6);
}

void KcmColorfulHelper::saveCSFile()
{

    QString newpath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            + "/color-schemes/";
    QProcess::execute(QString("bash -c \"rm '%1Colorful-*'\"").arg(newpath));
    QDir dir;
    dir.mkpath(newpath);
    newpath += colorSchemeName + ".colors";


    KConfig *config = mConfig->copyTo(newpath);
//    mConfig->markAsClean();
//    m_config->reparseConfiguration();
    KConfigGroup groupA(config, "General");
    groupA.writeEntry("Name", colorSchemeName);

    KConfigGroup group(config, "Colors:View");
    group.writeEntry("DecorationHover", *c);


//    KConfigGroup groupWMTheme(config, "WM");
    KConfigGroup groupWMOut(config, "WM");

    QStringList colorItemListWM;
    colorItemListWM << "activeBackground"
                    << "activeForeground"
                    << "inactiveBackground"
                    << "inactiveForeground";
//                    << "activeBlend"
//                    << "inactiveBlend";

    QVector<QColor> defaultWMColors;
    defaultWMColors << *c
                    << QColor(239,240,241)
                    << *c
                    << QColor(189,195,199)
/*                    << QColor(255,255,255)
                    << QColor(75,71,67)*/;

    int i = 0;
    for (const QString &coloritem : colorItemListWM)
    {
            groupWMOut.writeEntry(coloritem, defaultWMColors.value(i));
            ++i;
    }


    // sync it and delete pointer
    config->sync();
//    changeColorScheme(config);
    delete config;
    KSharedConfigPtr rConfig = KSharedConfig::openConfig(newpath);
    changeColorScheme(rConfig);

}

QColor KcmColorfulHelper::addJitter(QColor color)
{
    return QColor(color.red() + (qrand()%3 - 1), color.green() + (qrand()%3 - 1), color.blue() + (qrand()%3 - 1));
}

bool KcmColorfulHelper::isDarkTheme()
{
    if (c->red() < 80 && c->green() < 80 && c->blue() < 80) {
        return true;
    } else {
        return false;
    }
}

void KcmColorfulHelper::dealStdOut()
{
    while(!colorExtractProc->atEnd())
    {
        QString rgb(colorExtractProc->readLine());
        if (rgb.contains("IsoaSFlus")) {
            QStringList rgbList = rgb.split(",");
            c = new QColor(rgbList[1].toInt(), rgbList[2].toInt(), rgbList[3].toInt());
            qDebug() << c->red() << c->green() << c->blue();
//            saveCSFile();
            readTemplateCS();
            changeColorScheme(tConfig);
            save();
            QCoreApplication::exit();
        }
    }
}


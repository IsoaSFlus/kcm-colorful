#!/bin/sh
BASEDIR=$(cd `dirname $0`; pwd) # root of translatable sources
PROJECT="kcm_colorful"	# project name
WDIR="${BASEDIR}/po"	# working dir


echo "Preparing rc files"
cd ${BASEDIR}
# we use simple sorting to make sure the lines do not jump around too much from system to system
find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg'| sort > ${WDIR}/rcfiles.list
xargs --arg-file=${WDIR}/rcfiles.list extractrc > ${WDIR}/rc.cpp
cd ${WDIR}
echo "Done preparing rc files"


echo "Extracting messages"
cd ${BASEDIR}
# see above on sorting
find . -name '*.cpp' -o -name '*.h' -o -name '*.c'  -o -name '*.qml' | sort > ${WDIR}/infiles.list
echo "rc.cpp" >> ${WDIR}/infiles.list
cd ${WDIR}
xgettext --package-name=$PROJECT --add-comments --sort-output --msgid-bugs-address=me@isoasflus.com \
  --from-code=UTF-8 -C --kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
	-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
	--files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot || { echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages"


echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  echo $cat
  msgmerge -q --backup=none -U $cat ${PROJECT}.pot
done
echo "Done merging translations"


echo "Cleaning up"
cd ${WDIR}
rm rcfiles.list infiles.list rc.cpp
echo "All Done! :)"

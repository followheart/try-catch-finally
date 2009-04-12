# Front end for wvHtml
# Tested with wv-0.7.2 only
if [ "$1" = "" ]
then
	if [ "$YASE_FILENAME" = "" ]
	then
		echo "Error: filename parameter not supplied"
		echo "Usage: word2text <filename>"
		echo "       Output will be sent to stdout"
		exit 1
	else
		fname="$YASE_FILENAME"
	fi
else
	fname="$1"
fi
ext=`echo "$fname" | awk -F. '{print $NF}'`
if [ "$ext" = "gz" ]
then
	gunzip -c "$fname" > /tmp/yase1.$$
	fname=/tmp/yase1.$$
fi
if [ "$ext" = "zip" ]
then
	unzip -p "$fname" > /tmp/yase1.$$
	fname=/tmp/yase1.$$
fi
wvHtml --targetdir=/tmp "$fname" yase.$$
yasewvcnv /tmp/yase.$$
rm -f /tmp/yase.$$ /tmp/yase1.$$
exit 0

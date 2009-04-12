# Front end for pdftotext
if [ "$1" = "" ]
then
	echo "Error: filename parameter not supplied"
	echo "Usage: pdf2text <filename>"
	echo "       Output will be sent to stdout"
	exit 1
fi
/home/dibyendu/xpdf/bin/pdftotext "$1" /tmp/yase.$$
cat /tmp/yase.$$
rm -f /tmp/yase.$$
exit 0

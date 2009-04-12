# A sample text to xml converter
# Dibyendu Majumdar
# 21 March 2002 This now works for any text file
# 22 March 2002 Each document title now contains the first few words

if [ "$1" != "" ]
then
	FILENAME="$1"
else
	FILENAME="-"
fi
awk --assign=yase_logicalname="$YASE_LOGICALNAME" --assign=yase_filename="$YASE_FILENAME" --assign=yase_size="$YASE_SIZE" 'BEGIN {
	print "<?xml version=\"1.0\"?>"
	print "<!DOCTYPE YASEFILE SYSTEM \"yase.dtd\">"
	printf "<YASEFILE logicalname=\"%s\" filename=\"%s\" size=\"%s\">\n", yase_logicalname, yase_filename, yase_size
	line=0
	paragraph=1
	pnum=0
}
{
	line++
	if (paragraph==1) {
		if (NF == 0)
			next
		if (line != 1) {
			print "</YASEDOC>"
		}
		pnum++
		printf "<YASEDOC title=\"Paragraph %d at Line %d, beginning with [", pnum, line
		gsub(/["<>]/, "")
		for (i = 1; i <= NF; i++) {
			if (i > 8)
				break;
			printf " %s", $i
		}
		printf " ...]\">\n"
		paragraph = 0
	}
	if (NF == 0) {
		paragraph=1
	}
	else {
		print
	}
}
END {
	if (line > 0) {
		print "</YASEDOC>"
	}
	print "</YASEFILE>"
}' "$FILENAME"

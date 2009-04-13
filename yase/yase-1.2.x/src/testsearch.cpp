// 09-01-03: Moved out of boolsearch.cpp and rankedsearch.cpp
#include "search.h"

int Ys_debug = 0;
int main(int argc, const char *argv[])
{
	if (argc < 4) {
		fprintf(stderr, "usage: testsearch <path> <mode> <query>\n");
		exit(1);
	}

	Collection collection;
	if (collection.open(argv[1], "r") != 0)
		exit(1);

	Search *search = 0;
	if (argv[2][0] == 'b')
		search = Search::createSearch(&collection, Search::SM_BOOLEAN);
	else
		search = Search::createSearch(&collection, Search::SM_RANKED);
	search->addInput(argv[3]);
	search->parseQuery();
	SearchResultSet *rs = search->executeQuery();
	if (rs != 0) {
		SearchResultItem *item = rs->getNext();
		while (item != 0) {
			item->dump(stdout);
			item = rs->getNext();
		}
	}
	delete rs;
	delete search;
	return 0;
}

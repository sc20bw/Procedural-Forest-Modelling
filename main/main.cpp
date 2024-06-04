#include "forest.hpp";

int main() {
	int treesRow;
	int treesCol;
	int treeType;
	float branchlength;
	float radius1;
	int n;
	cout << "Number of trees in a row: ";
	cin >> treesRow;
	cout << "Number of trees in a column: ";
	cin >> treesCol;
	cout << "What type of tree: ";
	cin >> treeType;
	cout << "Intial branch length: ";
	cin >> branchlength;
	cout << "Intial stem length: ";
	cin >> radius1;
	cout << "Depth of fractals: ";
	cin >> n;

	createForest(treesRow, treesCol, treeType, branchlength, radius1, n);
	return 0;
}
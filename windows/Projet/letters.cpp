
#include<stddef.h>

char letters_index(size_t index) {
	char ret;
	switch (index) {
	case 26:
		ret = 'I';
		break;
	case 27:
		ret = 'M';
		break;
	case 28:
		ret = 'Q';
		break;
	case 29:
		ret = 'R';
		break;
	case 30:
		ret = 'U';
		break;
	case 31:
		ret = 'H';
		break;
	case 32:
		ret = 'O';
		break;
	case 33:
		ret = 'B';
		break;
	case 34:
		ret = 'H';
		break;
	case 35:
		ret = 'O';
		break;
	case 36:
		ret = 'E';
		break;
	case 37:
		ret = 'N';
		break;
	case 38:
		ret = 'D';
		break;
	case 39:
		ret = 'L';
		break;
	case 40:
		ret = 'M';
		break;
	case 41:
		ret = 'M';
		break;
	case 42:
		ret = 'C';
		break;
	case 43:
		ret = 'G';
		break;
	case 44:
		ret = 'S';
		break;
	case 45:
		ret = 'R';
		break;
	case 46:
		ret = 'P';
		break;
	case 47:
		ret = 'W';
		break;
	case 48:
		ret = 'I';
		break;
	case 49:
		ret = 'A';
		break;
	case 50:
		ret = 'F';
		break;
	case 51:
		ret = 'M';
		break;
	case 52:
		ret = 'E';
		break;
	case 53:
		ret = 'M';
		break;
	case 54:
		ret = 'I';
		break;
	case 55:
		ret = 'J';
		break;
	case 56:
		ret = 'M';
		break;
	case 57:
		ret = 'T';
		break;
	case 58:
		ret = 'E';
		break;
	case 59:
		ret = '|';
		break;
	case 60:
		ret = 'M';
		break;
	case 61:
		ret = 'Z';
		break;
	case 62:
		ret = 'S';
		break;
	case 63:
		ret = 'X';
		break;
	case 64:
		ret = 'L';
		break;
	case 65:
		ret = 'A';
		break;
	case 66:
		ret = 'D';
		break;
	case 67:
		ret = 'I';
		break;
	case 68:
		ret = 'O';
		break;
	case 69:
		ret = 'I';
		break;
	case 70:
		ret = 'I';
		break;
	case 71:
		ret = 'I';
		break;
	case 72:
		ret = 'M';
		break;
	case 73:
		ret = 'N';
		break;
	case 74:
		ret = 'B';
		break;
	case 75:
		ret = 'H';
		break;
	case 76:
		ret = 'O';
		break;
	case 77:
		ret = 'N';
		break;
	case 78:
		ret = 'N';
		break;
	case 79:
		ret = 'M';
		break;
	case 80:
		ret = 'D';
		break;
	case 81:
		ret = 'M';
		break;
	case 82:
		ret = 'D';
		break;
	case 83:
		ret = 'O';
		break;
	case 84:
		ret = 'R';
		break;
	default:
		ret = 'A' + index;
		break;
	}
	return ret;
}
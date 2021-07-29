#pragma once
// ERR_UNBALANCED_BRACES: There is an opening brace without a closing brace or
// vice versa.
#define ERR_UNBALANCED_BRACES "Unbalanced braces in %s"
// ERR_UNFINISHED_BLOCK_COMMENT: There is a block comment without a closing marker
#define ERR_UNFINISHED_BLOCK_COMMENT "Unfinished multiline comment near %s"
// ERR_SPECIFICATION_MISSING_PATH: A path is missing in the line
#define ERR_SPECIFICATION_MISSING_PATH "Missing output path at line %d in %s"
// ERR_SPECIFICATION_MULTIPLE_PATH: There is more than one path in the same line
#define ERR_SPECIFICATION_MULTIPLE_PATH "Too many paths at line %d in %s. Remember to escape right arrows if you wish to compare them"
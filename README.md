# rfScss

This tool parses and refactors CSS or SCSS files

## Why?

I've found myself in the need to convert a 28k+ lines SCSS file (compiled from a CSS) into many different files, because dealing with a file of this size is complicated for the editor.

## About

This utility allows you to refactor the main file, split it into multiple imports or select a specific subset and paste it in a different file, depending in the .rfscss specification

The .rfscss specification contains a set of rules that tell rfScss how to deal with the input code.

SCSS is parsed from a sequence of Unicode code points, without first being converted in a token stream. This utility, however, does not decode into Unicode, instead it directly deals with the sequence of bytes.
As of now, this utility does not support nesting due to lack of time.

## Usage

```
Spawn rfScss and provide the path to a .scss file as a first argument:
- rfscss <path>

The file will be analized and parsed. If a .rfscss file was not found, it will be generated automatically at the parent directory. This file will contain the default refactoring rules which you can customize after.

If a .rfscss file was found, the program will read it and set all of the rules specified in it.
```

## Constraints

For cleanliness, intrusive comments in the selector are stripped out always.

```scss
.selector/* comment */,
.another-selector {
    &-sub {
        color: red;
    }
}

.selector,
.another-selector {
    &-sub {
        color: red;
    }
}
```

# The .rfscss file

A _.rfscss_ file should contain a sequence of rules. These rules must consist of a _match string_ && an output file path, separated by a right arrow. This file is parsed at the moment of running rfscss && it is expected to be found at the same directory of the main _.scss_ file. A default _.rfscss_ file is generated if it does not exist.

The _match string_ is compared against the selectors. All characters must be exact, except for the _wildcards_ where the characters could be any.

The _wildcards_ are:

-   `%`: match zero, one or more characters.
-   `_`: match one character.
-   `?`: match && capture zero, one or more characters.
-   `\`: match the next character literally. (escape)

They resemble the `LIKE` SQL operator, but also introduce a `?` wildcard, used for capturing a part of the selector.

If it conflicts with a wildcard, you can use `\` to escape it. _Wildcards_ are not greedy by default, they will match all of the characters until the first occurrence of the next character.

The following is valid _.rfscss_ syntax:

```
\%%->base/placeholders.scss
@%->base/rules.scss
.fa-%->vendor/fontawesome.scss
.fas->vendor/fontawesome.scss
.Home%->pages/home.scss
%carousel%->components/carousel.scss
.?\ %->components/?.scss
%->base/misc.scss
```

These rules will match the following strings accordingly (multiple examples separated by comma):

```
%placeholder
@at-rule
.fa-arrow-right
.fas (it will only match .fas)
.Home, .Home-1, ...
#carousel, .carousel, carousel, [carousel=1], ...
.class_name .another_class_name (only class_name will be captured)
anything
```

## Constraints

The rules are ran in the order they are specified. **The order of the rules does matter**.

In case of multiple `?` _wildcards_ in the same _match string_, they are placed in the order they are captured.

If a selector does not match any of the rules specified, it gets assigned a file name && gets placed at the parent directory of the base _.scss_ file.

Keep in mind that the right arrow used to separate the rules **also counts as a special character.** If you wish to compare it against a selector you must escape it as `-\>`, otherwise you'll receive an error while parsing the _.rfscss_ file.

# rfscss

This tool allows you to extract style rules and refactor large stylesheets.

## Index

-   <a href="#about">About</a>
-   <a href="#usage">Usage</a>
-   <a href="#build">Build</a>
-   <a href="#notes">Notes</a>
-   <a href="#rfscss_file">The .rfscss file</a>
-   <a href="#constraints">Constraints</a>

## Why?

I've found myself in the need to convert a 28k+ lines SCSS file (compiled from a CSS) into many different files, because dealing with a file of this size is complicated for the editor.

<h3 id="about">About</h3>

This utility allows you to refactor the main file, split it into multiple imports or select a specific subset and paste it in a different file, depending in the .rfscss specification. It allows you to steal style rules from a stylesheet source.

The .rfscss specification contains a set of rules that tell rfscss how to deal with the input code. As of now, this utility does not support nested rules due to lack of time.

<h3 id="usage">Usage</h3>

There is a detailed tutorial available at <a href="https://coals.live/blog/refactor-large-scss-into-multiple-files">this article</a>

Spawn rfscss and provide the path to a .css/.scss file as a first argument:
`$ rfscss <path>`

The file will be analized and parsed. If a .rfscss file was not found, it will be generated automatically at the parent directory. This file contains the default refactoring rules which you are supposed to customize after.

<h3 id="build">Build</h3>

To build this utility, you will need CMake 3.12 or above, as well as `make`.

Run the following command to compile the project:
`cmake . && make`

Alternatively, there is a linux pre-compiled binary available at <a href="https://github.com/coalio/rfscss/releases">releases</a>

<h3 id="notes">Notes</h3>

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

<h3 id="rfscss_file">The .rfscss file</h3>

A _.rfscss_ file should contain a sequence of rules. These rules must consist of a _pattern_ and an output file path, separated by a right arrow. This file is parsed at the moment of running rfscss and it is expected to be found at the same directory of the main _.scss_ file. A default _.rfscss_ file is generated if it does not exist.

The _pattern_ is compared against the selectors. All characters must be exact, except for the _wildcards_ where the characters could be any.

The _wildcards_ are:

-   `%`: match zero, one or more characters.
-   `_`: match one character.
-   `?`: match and capture zero, one or more characters.
-   `\`: match the next character literally. (escape)

They resemble the `LIKE` SQL operator, but also introduce a `?` wildcard, used for capturing a part of the selector.

If it conflicts with a wildcard, you can use `\` to escape it. _Wildcards_ are not greedy, they will match all of the characters until the next sequence of characters is found.

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

<h3 id="constraints">Constraints</h3>

The rules are ran in the order they are specified. **The order of the rules does matter**.

In case of multiple `?` _wildcards_ in the same _pattern_, the captures are placed in the order they are captured.

If a selector does not match any of the patterns specified in the .rfscss file, this selector **will be ignored**. If you wish to include all selectors/rules, you must be either _very specific_ or include a `#` or `?` rule that will match anything _just in case._

Keep in mind that the right arrow used to separate the rules **also counts as a special character.** If you wish to compare it against a selector you must escape it as `-\>`, otherwise you'll receive an error while parsing the _.rfscss_ file.

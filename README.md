# rfscss

This tool allows you to extract style rules and refactor large stylesheets.

## Index

-   <a href="#about">About</a>
-   <a href="#usage">Usage</a>
-   <a href="#build">Build</a>
-   <a href="#notes">Notes</a>
-   <a href="#rfscss_file">The .rfscss file</a>
-   <a href="#constraints">Constraints</a>
-   <a href="#contributing">Contributing</a>

<h3 id="about">About</h3>

This utility allows you to refactor a SCSS/CSS stylesheet, split it into multiple files or extract a specific subset into in a different file, depending in the rfscss specification.

The rfscss specification contains a set of rules that tell rfscss how to deal with the input code. The rules are applied in the order they are defined.

<h3 id="usage">Usage</h3>

Example usage:

```
curl https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/css/bootstrap.min.css | rfscss -p -i ".nav%->bootstrap-nav.css"
```

There is a detailed guide available at <a href="https://tabe.me/blog/refactor-large-scss-into-multiple-files">this article</a>

If you run rfscss without arguments, the following information will be shown:

```
Usage: rfscss <file | -p> [ -l <path> | -i <rfscss> | -e <path> | -w | -t ]
  --from-pipe [-p]: read from stdin.
  --tidy [-t]: tidy the selector when using ? to extract into a file.
  --enable-warnings [-w]: enable warnings.
  --list [-l] <path>: list all of the selectors in a file.
  --inline-rfscss [-i] <rfscss>: use <rfscss> as the specification.
  --export-imports [-e] <path>: write @import rules for every extracted rule, and save it at <path>
```

`--from-pipe [-p]`: Reads the input scss/css stdin.

`--tidy [-t]`: The selector will be split into multiple segments separated by underscore, special characters removed.

`--enable-warnings [-w]`: rfscss will let you know if there is something that could go wrong.

`--list [-l]`: Parses all of the selectors and rules in the file. For clearness, `selector` refers to `(.+?)\s*[\{;]` at the root level.

`--inline-rfscss [-i]`: Use the rfscss provided in the command. Takes precedence over the .rfscss file.

`--export-imports [-e]`: Writes an @import rule for every single output file and saves it at the path specified.

rfscss does not create directories recursively, so every path you specify should already exist. However, rfscss will create the folder at the base directory if it does not exist. This assures a single level directory creation.

<h3 id="build">Build</h3>

To build and install this utility, you will need CMake 3.12 or above, as well as `make`.

```
git clone https://github.com/coalio/rfscss
cd rfscss
cd build
bash build.sh --build-install

```

`build.sh` also provides other utilities, for example:

```
# Assumes you already built the binary and you only want to install it
bash build.sh -i

# Only builds the binary
bash build.sh

# Runs tests
bash build.sh --test
```

Of course, `build.sh` always assumes you're running it from the `/build` directory.

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

<h3 id="rfscss_file">The rfscss syntax</h3>

A _rfscss_ specification should contain a sequence of rules. These rules must consist of a _pattern_ and an output file path, separated by a right arrow. A _.rfscss_ file containing the specification is expected to be found at the same directory of the main _.scss_ file. Alternatively, a _rfscss_ specification can be provided using the `--inline-rfscss` option.

The _pattern_ is compared against the selectors. All characters must be exact, except when you use _wildcards_.

The _wildcards_ are:

-   `%`: match zero, one or more characters.
-   `_`: match one character.
-   `?`: match and capture zero, one or more characters.
-   `\`: match the next character literally.

They resemble the `LIKE` SQL operator, but also introduce a `?` wildcard, used for capturing a part of the selector.

If it conflicts with a wildcard, you can use `\` to escape it (for example, `\%` will match `%`).
_Wildcards_ are not greedy, they will match all of the characters until the next sequence of characters is found.

The following is valid _rfscss_ syntax:

```
\%%->base/placeholders.scss
@%->base/at-rules.scss
.fa-%->vendor/fontawesome.scss
.fas->vendor/fontawesome.scss
.Home%->pages/home.scss
%carousel%->components/carousel.scss
.? %->components/?.scss
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

If a selector does not match any of the patterns specified in the rfscss, this selector **will be ignored**. If you wish to include all selectors/rules, you must be either _very specific_ or include a `%` or `?` rule that will match anything _just in case._

Keep in mind that the right arrow used to separate the rules **has a special meaning.** If you wish to compare it against a selector you must escape it as `-\>`.

<h3 id="contributing">Contributing</h3>

If you wish to contribute to this project, please refer to <a href="https://github.com/coalio/rfscss/blob/master/CONTRIBUTING.md">this document</a>.

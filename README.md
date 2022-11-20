# Inja CLI

A command-line interface to [Inja](https://pantor.github.io/inja/), a C++ string template library loosely inspired by [Jinja](https://jinja.palletsprojects.com/).

## Usage

```cmd
inja input_file_template.html output_file.html
```

With a JSON data file:

```cmd
inja -d data.json input_file_template.html output_file.html
# or
inja --data data.json input_file_template.html output_file.html
```

## Template Syntax

For a quick overview, [refer to this syntax cheat sheet](./syntax.html).

For more detailed information, refer to [the Inja documentation](https://pantor.github.io/inja/).

## License

[MIT](./LICENSE)

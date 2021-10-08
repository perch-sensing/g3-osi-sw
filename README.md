# Perch 3GRMC SW

## SW

TBD

## Diagrams

The diagrams are created with [PlantUML](https://plantuml.com/). To install this
plugin, type <kbd>Ctrl</kbd> + <kbd>P</kbd>. Then, enter `ext install plantuml`.

To enable rendering of PlantUML diagrams, you'll need to install the PlantUML
server. This is fairly trivial, and can be accomplished with the following
command (you'll need to have Docker installed):

```shell
docker run -d -p 8187:8080 --name plantml plantuml/plantuml-server:jetty
```

You also need to edit your `settings.json` file and add the following lines:

```json
"plantuml.server": "http://localhost:8187",
"plantuml.render": "PlantUMLServer",
```

If you changed the port or the host of the PlantUML server, edit
`plantuml.server` appropriately.

To open `settings.json`, type <kbd>Ctrl</kbd> + <kbd>P</kbd>, then enter:

```text
>Preferences: Open Settings (JSON)
```
# Documentation Generation

TODO: Add this into a CI process, to automatically generate documentation for each commit.

This system extracts **API information** from **C++ comments** and generates **static HTML documentation**.

---

## Usage

Run the following commands in your terminal:

```bash
python doc_tool.py extract
```

> Extracts documentation comments from source files.

```bash
python doc_tool.py generate
```

> Generates static HTML documentation pages.

```bash
python doc_tool.py all
```

> Runs both extract and generate in sequence.

---

## Documentation Tags Reference

### Class

| **Tag**        | **Description**           | **Example**                 |
| -------------- | ------------------------- | --------------------------- |
| `@class`       | Class name                | `@class Part`               |
| `@brief`       | Short description         | `@brief A physical 3D part` |
| `@description` | Long description          | `@description Parts are...` |
| `@inherits`    | Parent class              | `@inherits BasePart`        |
| `@example`     | Usage example             | `@example Usage example`    |
| `@deprecated`  | Marks class as deprecated | `@deprecated Use NewPart`   |
| `@internal`    | Marks internal-only API   | `@internal`                 |

---

### Property

| **Tag**        | **Description**              | **Example**                       |
| -------------- | ---------------------------- | --------------------------------- |
| `@property`    | Property name                | `@property Position`              |
| `@type`        | Data type                    | `@type Vector3`                   |
| `@description` | What it does                 | `@description The 3D position...` |
| `@default`     | Default value                | `@default Vector3.new(0, 0, 0)`   |
| `@readonly`    | Mark as read-only            | `@readonly`                       |
| `@example`     | Usage example                | `@example Usage example`          |
| `@deprecated`  | Marks property as deprecated | `@deprecated Use NewPosition`     |
| `@internal`    | Marks internal-only API      | `@internal`                       |

---

### Method

| **Tag**        | **Description**            | **Example**                       |
| -------------- | -------------------------- | --------------------------------- |
| `@method`      | Method name                | `@method GetMass`                 |
| `@param`       | Parameter                  | `@param name type - description`  |
| `@returns`     | Return type                | `@returns number`                 |
| `@description` | What it does               | `@description Calculates mass...` |
| `@example`     | Usage example              | `@example Usage example`          |
| `@deprecated`  | Marks method as deprecated | `@deprecated Use ComputeMass`     |
| `@internal`    | Marks internal-only API    | `@internal`                       |

---

### Event

| **Tag**        | **Description**           | **Example**                  |
| -------------- | ------------------------- | ---------------------------- |
| `@event`       | Event name                | `@event Touched`             |
| `@param`       | Event parameter           | `@param otherPart BasePart`  |
| `@description` | When it fires             | `@description Fires when...` |
| `@example`     | Usage example (see below) | `@example Usage example`     |
| `@deprecated`  | Marks event as deprecated | `@deprecated Use OnTouch`    |
| `@internal`    | Marks internal-only API   | `@internal`                  |

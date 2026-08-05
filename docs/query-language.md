# Query language

## Grammar (informal)

```
query   := or_expr
or_expr := and_expr ( OR and_expr )*
and_expr:= unary ( AND unary | unary )*   # juxtaposition = AND
unary   := NOT unary | primary
primary := TERM | "phrase terms" | ( query )
```

- `AND`, `OR`, `NOT` are case-insensitive keywords.
- Bare terms are tokenized like documents (lowercase; punctuation splits).
- Phrases are quoted; tokens inside are tokenized the same way.
- Precedence: `NOT` > `AND` > `OR`.

## Examples

| Query | Meaning |
|-------|---------|
| `cats milk` | docs with both terms |
| `cats AND milk` | same |
| `dogs OR honey` | either term |
| `cats AND NOT milk` | cats without milk |
| `(dogs OR honey) AND milk` | grouping |
| `"cats love"` | adjacent phrase |

## Errors

The parser returns `{offset, message}`, for example:

- `Empty query`
- `Unterminated phrase (missing '"')`
- `Unmatched '(' — expected ')'`
- `Empty phrase query`
- `Unexpected end of query` / `Unexpected ')'`

Offsets are 0-based into the original query string (after trimming is applied only for emptiness checks; quote scan uses the raw input).

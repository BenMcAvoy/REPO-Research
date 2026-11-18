# Defensive analysis: managed runtime tampering

## Scope

The prototype resolves Unity/Mono assemblies, classes, fields, and methods, inspects managed objects, renders overlays, and includes writes to managed game state. These capabilities illustrate risks when a game client is treated as a trusted source of gameplay state.

## Threat model

Assume a player can inspect and alter data inside their local client. An overlay may reveal locally available information, while changes made only to local state may be presented as if they were valid game actions.

## Defensive considerations

- Keep shared and competitive outcomes authoritative on the server. Validate state changes against server-owned rules and the history of the match.
- Send each client only the data it needs. Client-side concealment is not a reliable boundary once information is present in that process.
- Treat client integrity and behavior signals as supplementary. They can inform investigation, but should not be the only basis for enforcement.
- Design supported mod APIs with explicit permissions and clear separation between local presentation changes and shared game state.

## Limits

This is a version-dependent research prototype, not an anti-cheat and not a measured evaluation of R.E.P.O.'s protections. The repository does not establish whether a particular defensive control is present or effective in the game.

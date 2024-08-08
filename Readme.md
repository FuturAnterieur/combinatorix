# What is this project

Game engine mostly for turn-based war games, with a game planned to be added to the same repo.

## The three levels of status/parameter changes

- 'intrinsics' : the values as they currently are 'naturally', before applying the current status effects
- 'locals' : the values as they are modified by status effects.
  (relatively new) rule : locals are always calculated from the latest intrinsics, not on top of other local changes as they are being applied by the status effects.
  Hence history_manager::get_active returns the latest currents, and not a snapshot of the locals as they are being applied.

  Actually locals are stacked and saved aside during status effects application;
  when we have gone through all these effects, we merge the resulting vector of status effects, taking priority and incremental changes into account.

  Maybe future TO-DO : currently only one CommitterId can be saved per resulting-change-at-timing, even when many incremental changes have been retained.

- 'currents' : the combination of latest intrinsics + current status effects applied to them.

Storage for these three things is managed with the same logic, through history_storage;
the history_manager keeps history_storage objects for status effect changes.

## Status effect modifications
It also keeps storage for status effects changes (i.e. lists of status effect entities).
Currently change recording for status effects is pretty bare-bones. 
It might need, in the future : 
  - a Diff type, using minimum-edit-distance between list of status effects
  - generalizing history_storage
  - maybe more things...


## Change editing 
Currently, intrinsic changes can be intercepted and modified before they are applied.
- Only modification type that can be applied to Change objects is 'set', not 'incremental' 
- Changes can be banned through this mechanic, but for now, bans potentially affect all changes/edits on a given hash (barring priority); they are not selective.

# What is planned
## Client API
- Create separate APIs (all referring to game_logic) for status triggers, pre-change triggers, status effects, and the lambda passed to game_logic::run_simulation
- Offer capacity to edit triggers inside user callbacks mentioned just above

## Change editing
- Add change ban filter feature (a std::function in engine::change_suppression_edit).
  - don't forget to merge the edits logically after that (i.e. only block changes that    are catched by the filters that have higher priorities than them).
- Incremental modifications to existing changes
- Clean up the API (clients should not access change_edit_history directly).

## Status effect history
- See section above.


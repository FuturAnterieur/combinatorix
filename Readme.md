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
- Change edits are added to a temporary stack in the change edit history
- At the start, the original change (issued by the user) is placed on the stack.
- Only modification type that can be applied to Change objects is 'set', not 'incremental' 
- Changes can be banned through this mechanic.
- All change edits have a filter which decides if the change edit is added to the stack or not
- Then suppression type edits (i.e. bans) that are on the stack can have a filter that will be applied to each change in the stack, to determine if the ban 'catches' the edit or not.
- For a ban to suppress a change, the ban needs to have higher priority and a filter that catches the change.
- For now, bans cannot ban other bans.

# What is planned
## Client API
- Create separate APIs (all referring to game_logic) for status triggers, pre-change triggers, status effects, and the lambda passed to game_logic::run_simulation
- Offer capacity to edit triggers inside user callbacks mentioned just above

## Change editing
- <<====>>> DONE <<<====>>> Add change ban filter feature (a std::function in engine::change_suppression_edit).
  - don't forget to merge the edits logically after that (i.e. only block changes that    are catched by the filters that have higher priorities than them). ====>>
- Incremental modifications to existing changes
- Implement change editing for local (i.e. status effect) changes. Should use the same structures/functions that change-editing-for-intinsics uses.
- Clean up the API (clients should not access change_edit_history directly).

## Status effect history
- See section above.


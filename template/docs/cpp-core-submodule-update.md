# Updating cpp-core as a Git Submodule

Documentation root: [README.md](../README.md)

This guide has two parts:
- A concise checklist (quick reference)
- A detailed breakdown (meaning, effects, failure cases, and fixes)

## Concise Checklist

1. In `third_party/cpp-core`, update code on `main`, then commit and push.
2. Create and push a release tag on that commit (for example `vX.Y.Z`).
3. In the parent repo, move the submodule to that tag/commit:
   - `git -C third_party/cpp-core fetch --tags`
   - `git -C third_party/cpp-core checkout vX.Y.Z`
4. In the parent repo, verify:
   - `sh ci/verify_cpp_core_submodule.sh`
   - `git submodule status third_party/cpp-core`
5. Commit the submodule pointer update in the parent repo and push.

## Detailed Breakdown

### Step 1: Update cpp-core on main, commit, and push

What to do:
```sh
git -C third_party/cpp-core checkout main
git -C third_party/cpp-core pull --ff-only origin main
# make changes
git -C third_party/cpp-core add -A
git -C third_party/cpp-core commit -m "Your change summary"
git -C third_party/cpp-core push origin main
```

Meaning/effect:
- You are creating a new commit in the `cpp-core` repository itself.
- Nothing in the parent project is updated yet.

What can go wrong:
- You are not on `main`.
- Local branch is behind/diverged.
- Commit fails due to hooks/lint/tests.
- Push rejected due to remote updates.

How to fix:
- Switch and sync branch:
  - `git -C third_party/cpp-core checkout main`
  - `git -C third_party/cpp-core pull --ff-only origin main`
- If push is rejected, pull/rebase and push again:
  - `git -C third_party/cpp-core pull --rebase origin main`
  - resolve conflicts, then `git -C third_party/cpp-core push origin main`
- If checks fail, fix code/tests, then recommit.

### Step 2: Create and push a release tag for that commit

What to do:
```sh
git -C third_party/cpp-core tag -a vX.Y.Z -m "Release vX.Y.Z"
git -C third_party/cpp-core push origin vX.Y.Z
```

Meaning/effect:
- You mark a specific commit as an official release.
- The parent project can now pin to a stable, named release.

What can go wrong:
- Tag already exists.
- Tag points to the wrong commit.
- Push tag denied by permissions/policy.

How to fix:
- Check current commit and existing tags:
  - `git -C third_party/cpp-core rev-parse HEAD`
  - `git -C third_party/cpp-core tag --list 'v*' --sort=-v:refname | head`
- If tag is wrong and not yet consumed, delete/recreate carefully:
  - local delete: `git -C third_party/cpp-core tag -d vX.Y.Z`
  - remote delete: `git -C third_party/cpp-core push origin :refs/tags/vX.Y.Z`
  - recreate and push again
- If policy blocks rewriting tags, create a new incremented tag instead (preferred in protected repos).

### Step 3: Move parent repo submodule checkout to that release

What to do:
```sh
git -C third_party/cpp-core fetch --tags origin
git -C third_party/cpp-core checkout vX.Y.Z
```

Meaning/effect:
- This updates your local submodule working tree to the release commit.
- Parent repo now sees `third_party/cpp-core` as modified (gitlink change pending).

What can go wrong:
- `pathspec 'vX.Y.Z' did not match any file(s) known to git`.
- Detached HEAD warning (expected here).
- You accidentally checked out the wrong tag/commit.

How to fix:
- Fetch tags and verify existence:
  - `git -C third_party/cpp-core fetch --tags origin`
  - `git -C third_party/cpp-core tag --list 'v*'`
- Detached HEAD is normal for pinned submodule releases.
- Verify exact target before continuing:
  - `git -C third_party/cpp-core describe --tags --exact-match HEAD`
  - `git -C third_party/cpp-core rev-parse HEAD`

### Step 4: Verify policy and pin state in parent repo

What to do:
```sh
sh ci/verify_cpp_core_submodule.sh
git submodule status third_party/cpp-core
git ls-tree HEAD third_party/cpp-core
```

Meaning/effect:
- Confirms the submodule commit is on a release tag (per CI script).
- Confirms local submodule state and currently committed parent gitlink.

What can go wrong:
- Verify script says "not pinned to a release tag".
- Submodule not initialized/missing.
- Local state and committed gitlink do not match yet.

How to fix:
- Ensure you checked out a valid release tag in submodule.
- Initialize if needed:
  - `git submodule update --init --recursive third_party/cpp-core`
- If committed gitlink is still old, proceed to Step 5 and commit the pointer update.

### Step 5: Commit and push the parent repo submodule pointer update

What to do:
```sh
git add third_party/cpp-core
git commit -m "Update cpp-core submodule to vX.Y.Z"
git push
```

Meaning/effect:
- This records the new submodule commit ID (gitlink) in the parent repository history.
- Teammates/CI will get the same cpp-core version when they sync submodules.

What can go wrong:
- You forget this step, so update exists only locally.
- Wrong submodule commit gets recorded.
- CI fails due to policy or missing tag.

How to fix:
- Confirm staged pointer before committing:
  - `git diff --cached --submodule=short -- third_party/cpp-core`
- If wrong pointer committed, check out correct tag in submodule and commit again.
- If CI fails, rerun verification locally:
  - `sh ci/verify_cpp_core_submodule.sh`

## Fast Validation Commands

Use this small set after you think you are done:
```sh
git submodule status third_party/cpp-core
git -C third_party/cpp-core describe --tags --exact-match HEAD
sh ci/verify_cpp_core_submodule.sh
git status --short
```

Expected outcome:
- Submodule shows the intended tagged commit.
- Verify script passes.
- Parent repo is clean after committing the pointer update.

# Using MekkoGX as a Subtree

This guide explains how to use **only the `makefiles/` directory** from MekkoGX in your own project — without using git submodules.

## Step 1: Create a “makefiles-only” branch

From the MekkoGX clone:

```
git checkout main
git subtree split --prefix=makefiles -b makefiles-only
```

Push it:

```
git push origin makefiles-only
```

## Step 2: Add the subtree

From your project repo:

```
git subtree add --prefix=makefiles https://github.com/FozzTexx/mekkogx.git makefiles-only --squash
```

## Step 3: Pull updates

```
git subtree pull --prefix=makefiles https://github.com/FozzTexx/mekkogx.git makefiles-only --squash
```

## Step 4: Push fixes back

```
git subtree push --prefix=makefiles https://github.com/FozzTexx/mekkogx.git makefiles-only
```

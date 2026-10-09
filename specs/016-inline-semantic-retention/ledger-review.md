# Individual ledger review

Reviewed on 2026-10-09 against pinned source/HTML, independently passing semantic comparisons and pre/post presentation fields. No expected tree was generated from production output. All 652 parser/model comparisons pass; source-authored fixtures were separately reviewed from source. All 134 former loss-bearing IDs contain the independently compared retained distinctions outside opaque image interiors. Each row records why its exception changes. Existing limits are never removed. Fingerprints change only because the raw model includes semantic trees; entries with no remaining limit are removed. Unchanged entries are omitted.

The two added limits expose pre-existing source ambiguity: generated semantic HTML cannot identify literal source HTML tokens, and a serialized LF may originate from an entity rather than a soft break. These limits do not mask nodes or flatten containers/breaks. Source-authored cases and dedicated parser tests check exact distinctions.

| Example | Ledger action | Evidence reviewed |
| --- | --- | --- |
| 5 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 14 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 15 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 16 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 17 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 20 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 22 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 23 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 25 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 28 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 32 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 33 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 35 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 37 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 46 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 49 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 56 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 66 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 70 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 80 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 81 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 82 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 87 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 88 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 93 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 95 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 104 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 105 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 106 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 109 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 113 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 114 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 115 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 121 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 128 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 138 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 140 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 141 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 145 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 168 | retained | Independent html-projection verifies literal inline HTML identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 192 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 193 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 194 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 195 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 196 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 198 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 200 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 201 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 202 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 203 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 204 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 205 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 206 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 211 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 212 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 213 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 214 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 215 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 216 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 217 | retained | Independent html-projection verifies link titles, soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 218 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 220 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 222 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 223 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 224 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 225 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 226 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 228 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 229 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 230 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 232 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 233 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 237 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 238 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 243 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 247 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 250 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 251 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 252 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 253 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 254 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 257 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 259 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 263 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 264 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 265 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 267 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 268 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 270 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 271 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 272 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 273 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 274 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 278 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 283 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 285 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 286 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 287 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 288 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 290 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 291 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 292 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 293 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 296 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 297 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 299 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 302 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 304 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 305 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 311 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 312 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 313 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 318 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 321 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 324 | retained | Independent tree equality passes; existing limits retained; semantic fingerprint refreshed. |
| 327 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 328 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 329 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 330 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 331 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 332 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 333 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 334 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 335 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 336 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 337 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 338 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 339 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 340 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 341 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 342 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 343 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 344 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 345 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 346 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 349 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 350 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 355 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 356 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 357 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 364 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 367 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 369 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 370 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 373 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 376 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 377 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 378 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 381 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 382 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 384 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 389 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 390 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 393 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 394 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 395 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 396 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 399 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 402 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 403 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 404 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 405 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 406 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 407 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 408 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 409 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 410 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 411 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 412 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 413 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 414 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 415 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 416 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 417 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 418 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 419 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 422 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 423 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 424 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 425 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 426 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 427 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 428 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 429 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 430 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 431 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 432 | retained | Independent html-projection verifies repeated emphasis/strong nesting, soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 433 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 437 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 438 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 440 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 441 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 442 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 443 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 444 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 445 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 446 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 447 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 449 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 450 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 452 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 453 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 454 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 455 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 456 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 457 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 458 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 459 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 460 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 461 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 462 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 463 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 464 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 465 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 466 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 467 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 468 | retained | Independent html-projection verifies repeated emphasis/strong nesting in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 469 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 470 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 471 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 472 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 473 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 474 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 475 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 476 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 477 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 478 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 479 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 480 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 481 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 482 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 483 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 484 | retained | Independent html-projection verifies empty link containers in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 485 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 486 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 487 | retained | Independent html-projection verifies empty link containers in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 489 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 490 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 491 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 492 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 494 | removed | Independent source-authored verifies literal inline HTML identity, soft-break identity in retained nodes |
| 495 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 496 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 498 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 499 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 500 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 501 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 502 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 503 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 504 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 505 | retained | Independent html-projection verifies link titles, soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 506 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 507 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 509 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 510 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 512 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 514 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 515 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 516 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 517 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 518 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 519 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 520 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 521 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 522 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 523 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 524 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 525 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 526 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 527 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 528 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 529 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 530 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 531 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 532 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 533 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 534 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 535 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 536 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 537 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 538 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 539 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 540 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 541 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 542 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 543 | retained | Independent html-projection verifies link titles, soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable |
| 544 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 549 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 550 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 552 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 553 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 554 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 555 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 556 | retained | Independent html-projection verifies link titles, soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable; Source review: trailing spaces trim to an empty cmark Text artifact, omitted by adaptation; SoftBreak and Link/Image containers retained. |
| 557 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 558 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 559 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 560 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 561 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 562 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 564 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 565 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 566 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 567 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 568 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 569 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 570 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 571 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 572 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 573 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 574 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 575 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 576 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 577 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 578 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 579 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 580 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 581 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 582 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 583 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 584 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 585 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 586 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 587 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable, softbreak-or-decoded-lf-source-unavailable; Source review: trailing spaces trim to an empty cmark Text artifact, omitted by adaptation; SoftBreak and Link/Image containers retained. |
| 588 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 589 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 591 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 593 | retained | Independent html-projection verifies link titles in retained nodes; Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 594 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 595 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 596 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 597 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 598 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 599 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 600 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 601 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 603 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 604 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 605 | retained | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 613 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 614 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 615 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 616 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 617 | removed | Independent html-projection verifies literal inline HTML identity in retained nodes |
| 621 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 623 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 625 | removed | Independent html-projection verifies literal inline HTML identity in retained nodes |
| 626 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 627 | removed | Independent html-projection verifies literal inline HTML identity in retained nodes |
| 628 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 629 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 630 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 631 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 633 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 634 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 635 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 636 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 637 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 638 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 639 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 640 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 641 | added | Explicit ambiguity: inline-html-or-generated-tag-source-unavailable |
| 642 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 643 | removed | Independent source-authored verifies literal inline HTML identity in retained nodes |
| 648 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |
| 649 | retained | Independent html-projection verifies soft-break identity in retained nodes; Explicit ambiguity: softbreak-or-decoded-lf-source-unavailable |

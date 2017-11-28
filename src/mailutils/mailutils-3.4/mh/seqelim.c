/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2017 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#include <mh.h>

struct seq_closure
{
  mu_msgset_t rmset;
  int rmflag;
};

static int
seqelim (const char *name, const char *value, void *data)
{
  struct seq_closure *s = data;
  mu_mailbox_t mbox;

  mu_msgset_sget_mailbox (s->rmset, &mbox);
  mh_seq_delete (mbox, name, s->rmset, s->rmflag);
  return 0;
}

/* Eliminate given messages from all sequences */
void
mh_sequences_elim (mu_msgset_t msgset)
{
  struct seq_closure clos;
  mu_mailbox_t mbox;

  MU_ASSERT (mu_msgset_sget_mailbox (msgset, &mbox));
  clos.rmset = msgset;
  clos.rmflag = 0;
  mh_global_sequences_iterate (mbox, seqelim, &clos);
  clos.rmflag = SEQ_PRIVATE;
  mh_private_sequences_iterate (mbox, seqelim, &clos);
}

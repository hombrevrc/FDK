﻿namespace SoftFX.Extended.Financial
{
    using System;
    using System.Threading;
    using SoftFX.Extended.Events;
    using TickTrader.Common;

    sealed class Processor
    {
        readonly Action action;
        readonly EventWaitHandle actionEvent;

        bool wokeUp;
        Thread thread;

        public long Generation { get; private set; }

        public Processor(Action action)
        {
            this.action = action;
            this.actionEvent = new AutoResetEvent(false);
        }

        public void Start()
        {
            if (this.thread != null)
                return;

            this.thread = new Thread(this.Loop);
            this.thread.Start(this.thread);
        }

        public void Stop()
        {
            if (this.thread == null)
                return;

            this.thread = null;
            this.WakeUp();
        }

        public void WakeUp()
        {
            if (!this.wokeUp)
            {
                this.actionEvent.Set();
                this.wokeUp = true;
            }
        }

        public void EndWakeUp()
        {
            this.wokeUp = false;
        }

        void Loop(object arg)
        {
            for (this.actionEvent.WaitOne(); this.thread == arg; this.actionEvent.WaitOne())
            {
                this.StepSafe();
            }

            this.WakeUp();
        }

        void StepSafe()
        {
            try
            {
                this.Step();
            }
            catch (Exception ex)
            {
                Events.Raise(this.Exception, this, () => new ExceptionEventArgs(ex));
            }
        }

        void Step()
        {
            this.action();
            this.Generation++;
            Events.Raise(this.Executed, this);
        }

        public event EventHandler Executed;

        public event EventHandler<ExceptionEventArgs> Exception;
    }
}

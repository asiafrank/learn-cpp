package com.asiafrank.learn.springboot.core.vo;

import com.asiafrank.learn.springboot.core.model.Sample;
import com.asiafrank.learn.springboot.core.base.Expression;
import com.asiafrank.learn.springboot.core.base.ExpressionChain;

import java.util.ArrayList;
import java.util.List;

public class SampleVO extends Sample {
    private static final long serialVersionUID = 1L;

    private List<ExpressionChain> expressionChainList;

    public SampleVO() {
        expressionChainList = new ArrayList<ExpressionChain>();
    }

    public SampleVO or(ExpressionChain expressionChain) {
        expressionChainList.add(expressionChain);
        return this;
    }

    public SampleVO or(Expression expression) {
        expressionChainList.add(new ExpressionChain().and(expression));
        return this;
    }

    public SampleVO and(Expression expression) {
        if (expressionChainList.isEmpty()) {
            expressionChainList.add(new ExpressionChain());
        }
        expressionChainList.get(0).and(expression);
        return this;
    }

    public List<ExpressionChain> getExpressionChainList() {
        return expressionChainList;
    }

    public void setExpressionChainList(List<ExpressionChain> expressionChainList) {
        this.expressionChainList = expressionChainList;
    }
}

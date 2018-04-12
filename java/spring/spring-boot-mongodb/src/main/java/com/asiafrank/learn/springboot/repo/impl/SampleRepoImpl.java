package com.asiafrank.learn.springboot.repo.impl;

import com.asiafrank.learn.springboot.model.Sample;
import com.asiafrank.learn.springboot.repo.AbstractCustom;
import com.asiafrank.learn.springboot.repo.SampleRepoCustom;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;

import java.util.List;

/**
 * SampleRepoImpl
 * <p>
 * </p>
 * Created at 12/23/2016.
 *
 * @author asiafrank
 */
public class SampleRepoImpl extends AbstractCustom implements SampleRepoCustom {
    @Override
    public List<Sample> find(List<String> ids) {
        Query query = new Query();
        query.addCriteria(Criteria.where("_id").in(ids));
        return getOperations().find(query, Sample.class);
    }
}